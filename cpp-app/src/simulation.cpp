#include "simulation.hpp"

#include <algorithm>
#include <cmath>
#include <numeric>

namespace smartflow {
namespace {
constexpr double PI = 3.14159265358979323846;
constexpr double STOP_MARGIN = VEHICLE_LENGTH / 2 + 4;
const double SAFE_DISTANCE = std::hypot(VEHICLE_LENGTH, VEHICLE_WIDTH) + 7;
constexpr std::array<Vec2, 4> STARTS{{{540, -60}, {1060, 440}, {460, 860}, {-60, 360}}};
constexpr std::array<Vec2, 4> ENDS{{{460, -70}, {1070, 360}, {540, 870}, {-70, 440}}};
constexpr std::array<Vec2, 4> STOP_LINES{{{540, 270}, {630, 440}, {460, 530}, {370, 360}}};

std::size_t approachIndex(char approach) {
    const auto found = std::find(APPROACHES.begin(), APPROACHES.end(), approach);
    return static_cast<std::size_t>(found - APPROACHES.begin());
}
double distance(Vec2 a, Vec2 b) { return std::hypot(a.x - b.x, a.y - b.y); }

// A quarter-turn in screen coordinates rotates clockwise around the junction.
Vec2 rotate(Vec2 point, std::size_t quarterTurns) {
    double x = point.x - WORLD.cx;
    double y = point.y - WORLD.cy;
    for (std::size_t i = 0; i < quarterTurns; ++i) {
        const double oldX = x;
        x = -y;
        y = oldX;
    }
    return {x + WORLD.cx, y + WORLD.cy};
}
Vec2 cubic(const std::array<Vec2, 4>& p, double t) {
    const double u = 1 - t;
    return {
        u*u*u*p[0].x + 3*u*u*t*p[1].x + 3*u*t*t*p[2].x + t*t*t*p[3].x,
        u*u*u*p[0].y + 3*u*u*t*p[1].y + 3*u*t*t*p[2].y + t*t*t*p[3].y
    };
}
Route createRoute(char approach, const std::string& turn) {
    const std::size_t index = approachIndex(approach);
    const std::size_t destinationIndex = (index + (turn == "straight" ? 2 : turn == "left" ? 1 : 3)) % 4;
    constexpr double k = 0.5522847498307936;
    std::array<Vec2, 4> base;
    if (turn == "left") base = {{{540, 290}, {540, 290 + 70*k}, {610 - 70*k, 360}, {610, 360}}};
    else if (turn == "right") base = {{{540, 290}, {540, 290 + 150*k}, {390 + 150*k, 440}, {390, 440}}};
    else base = {{{540, 290}, {540, 363.333333}, {540, 436.666667}, {540, 510}}};
    Route route;
    route.id = std::string(1, approach) + "-" + turn;
    route.approach = approach;
    route.turn = turn;
    route.destination = APPROACHES[destinationIndex];
    route.start = STARTS[index];
    route.end = ENDS[destinationIndex];
    for (std::size_t i = 0; i < base.size(); ++i) route.curve[i] = rotate(base[i], index);
    std::vector<Vec2> points{route.start, route.curve[0]};
    for (int i = 1; i <= 100; ++i) points.push_back(cubic(route.curve, i / 100.0));
    points.push_back(route.end);
    route.samples.push_back({0, route.start.x, route.start.y});
    for (std::size_t i = 1; i < points.size(); ++i) {
        route.samples.push_back({route.samples.back().s + distance(points[i - 1], points[i]), points[i].x, points[i].y});
    }
    route.length = route.samples.back().s;
    route.entryProgress = route.samples[1].s;
    route.exitProgress = route.samples[route.samples.size() - 2].s;
    route.stopProgress = distance(route.start, STOP_LINES[index]);
    route.clearanceProgress = route.exitProgress + VEHICLE_LENGTH + 20;
    return route;
}
} // namespace

Pose Route::pointAt(double progress) const {
    const double s = std::clamp(progress, 0.0, length);
    std::size_t low = 1, high = samples.size() - 1;
    while (low < high) {
        const std::size_t mid = (low + high) >> 1;
        if (samples[mid].s < s) low = mid + 1;
        else high = mid;
    }
    const auto& previous = samples[low - 1];
    const auto& next = samples[low];
    const double interval = next.s - previous.s;
    const double ratio = (s - previous.s) / (interval == 0 ? 1 : interval);
    return {previous.x + (next.x - previous.x)*ratio,
            previous.y + (next.y - previous.y)*ratio,
            std::atan2(next.y - previous.y, next.x - previous.x)*180/PI};
}

const std::vector<Route>& routes() {
    static const std::vector<Route> values = [] {
        std::vector<Route> result;
        for (char approach : APPROACHES) {
            for (const char* turn : {"straight", "left", "right"}) result.push_back(createRoute(approach, turn));
        }
        return result;
    }();
    return values;
}

TrafficSimulation::TrafficSimulation(std::uint32_t seedValue, double densityValue, double greenDurationValue)
    : seed(seedValue),
      density(std::clamp(std::isnan(densityValue) || densityValue == 0 ? 24.0 : densityValue, 4.0, 60.0)),
      greenDuration(std::clamp(std::isnan(greenDurationValue) || greenDurationValue == 0 ? 10.0 : greenDurationValue, 6.0, 30.0)) {
    reset();
}

void TrafficSimulation::reset() {
    randomState_ = seed;
    accumulator_ = 0;
    ticks_ = 0;
    time = 0;
    phase = "green";
    phaseApproach = 'N';
    activeGreenDuration_ = greenDuration;
    phaseElapsed_ = 0;
    nextId_ = 1;
    completed_ = 0;
    completedWait_ = 0;
    completedTimes_.clear();
    pending_.clear();
    arrivalClock_ = 0;
    arrivalDelay_ = nextArrivalDelay();
    vehicles.clear();
    recentHistory.clear();
    lastHistorySecond_ = -1;
    refreshState();
}

void TrafficSimulation::setDensity(double number) {
    if (!std::isfinite(number)) return;
    const double previous = density;
    density = std::clamp(number, 4.0, 60.0);
    arrivalDelay_ = arrivalClock_ + (arrivalDelay_ - arrivalClock_) * previous / density;
}
void TrafficSimulation::setGreenDuration(double number) {
    if (std::isfinite(number)) greenDuration = std::clamp(number, 6.0, 30.0);
}
void TrafficSimulation::requestNextPhase() {
    if (phase == "green") {
        phase = "amber";
        phaseElapsed_ = 0;
        refreshState();
    }
}
std::string TrafficSimulation::signal(char approach) const {
    return approach == phaseApproach && phase != "allRed" ? phase : "red";
}
bool TrafficSimulation::intersectionOccupied() const {
    return std::any_of(vehicles.begin(), vehicles.end(), [](const Vehicle& vehicle) {
        return vehicle.committed && vehicle.progress < routes()[vehicle.routeIndex].clearanceProgress;
    });
}
std::string TrafficSimulation::phaseLabel() const {
    if (phase == "allRed") {
        return phaseRemaining <= 0 && intersectionOccupied() ? "Waiting for intersection to clear" : "All red - clearance";
    }
    constexpr std::array<const char*, 4> names{{"North", "East", "South", "West"}};
    return std::string(names[approachIndex(phaseApproach)]) + " - " + (phase == "green" ? "green" : "amber");
}
void TrafficSimulation::step(double dt) {
    if (!std::isfinite(dt) || dt <= 0) return;
    accumulator_ += dt;
    while (accumulator_ + 1e-10 >= FIXED_STEP) {
        fixedStep();
        accumulator_ -= FIXED_STEP;
    }
    if (accumulator_ < 0) accumulator_ = 0;
}

// uint32_t overflow is defined modulo 2^32, matching JS Math.imul/bitwise mixing.
double TrafficSimulation::random() {
    randomState_ += UINT32_C(0x6d2b79f5);
    std::uint32_t value = (randomState_ ^ (randomState_ >> 15)) * (UINT32_C(1) | randomState_);
    value ^= value + (value ^ (value >> 7)) * (UINT32_C(61) | value);
    return static_cast<double>(value ^ (value >> 14)) / 4294967296.0;
}
double TrafficSimulation::nextArrivalDelay() { return 60 / density * (0.55 + random()*0.9); }
void TrafficSimulation::refreshState() {
    const double duration = phase == "green" ? activeGreenDuration_ : phase == "amber" ? YELLOW_DURATION : ALL_RED_DURATION;
    phaseRemaining = std::max(0.0, duration - phaseElapsed_);
    stats.active = static_cast<int>(vehicles.size());
    stats.queued = static_cast<int>(std::count_if(vehicles.begin(), vehicles.end(), [](const Vehicle& vehicle) {
        return !vehicle.committed && vehicle.speed < 3 && vehicle.progress > 45;
    }));
    stats.completed = completed_;
    stats.averageWait = completed_ ? completedWait_ / completed_ : 0;
    stats.throughput = time ? completed_ / time * 60 : 0;
    stats.pending = static_cast<int>(pending_.size());
    stats.lastMinuteThroughput = static_cast<int>(completedTimes_.size());
}
void TrafficSimulation::advanceSignals() {
    phaseElapsed_ += FIXED_STEP;
    if (phase == "green" && phaseElapsed_ + 1e-9 >= activeGreenDuration_) {
        phase = "amber";
        phaseElapsed_ = 0;
    } else if (phase == "amber" && phaseElapsed_ + 1e-9 >= YELLOW_DURATION) {
        phase = "allRed";
        phaseElapsed_ = 0;
    } else if (phase == "allRed" && phaseElapsed_ + 1e-9 >= ALL_RED_DURATION && !intersectionOccupied()) {
        phaseApproach = APPROACHES[(approachIndex(phaseApproach) + 1) % APPROACHES.size()];
        phase = "green";
        activeGreenDuration_ = greenDuration;
        phaseElapsed_ = 0;
    }
}
void TrafficSimulation::spawnArrivals() {
    arrivalClock_ += FIXED_STEP;
    if (arrivalClock_ >= arrivalDelay_) {
        const char approach = APPROACHES[static_cast<std::size_t>(std::floor(random()*4))];
        const double turnChoice = random();
        pending_.push_back({approach, turnChoice < 0.15 ? "left" : turnChoice > 0.85 ? "right" : "straight"});
        arrivalClock_ -= arrivalDelay_;
        arrivalDelay_ = nextArrivalDelay();
    }
    for (std::size_t i = 0; i < pending_.size() && vehicles.size() < 180;) {
        const auto& request = pending_[i];
        const auto routeIt = std::find_if(routes().begin(), routes().end(), [&](const Route& candidate) {
            return candidate.approach == request.approach && candidate.turn == request.turn;
        });
        const std::size_t routeIndex = static_cast<std::size_t>(routeIt - routes().begin());
        const Pose point = routeIt->pointAt(0);
        const bool blocked = std::any_of(vehicles.begin(), vehicles.end(), [&](const Vehicle& other) {
            return std::hypot(other.x - point.x, other.y - point.y) < VEHICLE_LENGTH + MIN_GAP + 8;
        });
        if (blocked) { ++i; continue; }
        Vehicle vehicle;
        vehicle.id = nextId_++;
        vehicle.routeIndex = routeIndex;
        vehicle.approach = request.approach;
        vehicle.turn = request.turn;
        vehicle.destination = routeIt->destination;
        vehicle.x = point.x;
        vehicle.y = point.y;
        vehicle.angle = point.angle;
        vehicle.speed = MAX_SPEED;
        vehicle.colorIndex = (vehicle.id - 1) % 7;
        vehicle.bornAt = time;
        vehicles.push_back(vehicle);
        pending_.erase(pending_.begin() + static_cast<std::ptrdiff_t>(i));
    }
}
double TrafficSimulation::leaderGap(const Vehicle& vehicle, const Vehicle& other) const {
    if (&vehicle == &other) return -1;
    const auto& route = routes()[vehicle.routeIndex];
    const auto& otherRoute = routes()[other.routeIndex];
    if (vehicle.routeIndex == other.routeIndex || (vehicle.approach == other.approach && vehicle.progress < route.entryProgress && other.progress < otherRoute.entryProgress + 25)) {
        const double gap = other.progress - vehicle.progress;
        return gap > 0 ? gap : -1;
    }
    if (vehicle.destination == other.destination && vehicle.progress >= route.exitProgress && other.progress >= otherRoute.exitProgress) {
        const double heading = vehicle.angle * PI / 180;
        const double ahead = (other.x - vehicle.x)*std::cos(heading) + (other.y - vehicle.y)*std::sin(heading);
        return ahead > 0 ? ahead : -1;
    }
    return -1;
}
void TrafficSimulation::moveVehicle(Vehicle& vehicle) {
    const auto& route = routes()[vehicle.routeIndex];
    const double stop = route.stopProgress - STOP_MARGIN;
    const bool permitted = phase == "green" && phaseApproach == vehicle.approach;
    double targetSpeed = vehicle.turn != "straight" && vehicle.progress > route.entryProgress - 60 && vehicle.progress < route.exitProgress ? TURN_SPEED : MAX_SPEED;
    if (!vehicle.committed && !permitted) {
        const double gap = std::max(0.0, stop - vehicle.progress);
        targetSpeed = std::min(targetSpeed, std::sqrt(2*BRAKING*gap));
    }
    for (const auto& other : vehicles) {
        const double gap = leaderGap(vehicle, other);
        if (gap >= 0) {
            const double available = std::max(0.0, gap - VEHICLE_LENGTH - MIN_GAP);
            targetSpeed = std::min(targetSpeed, std::sqrt(other.speed*other.speed + 2*BRAKING*available));
            if (available < 1) targetSpeed = std::min(targetSpeed, other.speed);
        }
    }
    const double speed = vehicle.speed + std::clamp(targetSpeed - vehicle.speed, -BRAKING*FIXED_STEP, ACCELERATION*FIXED_STEP);
    double nextProgress = std::min(route.length, vehicle.progress + speed*FIXED_STEP);
    if (!vehicle.committed && !permitted) nextProgress = std::min(nextProgress, stop);
    nextProgress = std::max(vehicle.progress, nextProgress);
    Pose nextPoint = route.pointAt(nextProgress);
    const auto wouldOverlap = [&](Pose point) {
        return std::any_of(vehicles.begin(), vehicles.end(), [&](const Vehicle& other) {
            return &other != &vehicle && std::hypot(point.x - other.x, point.y - other.y) < SAFE_DISTANCE;
        });
    };
    // Move leaders first. A conservative circumscribed-circle test covers turns
    // and outgoing merges as well as same-lane following; never move backwards.
    if (wouldOverlap(nextPoint)) {
        double low = vehicle.progress, high = nextProgress;
        for (int i = 0; i < 9; ++i) {
            const double mid = (low + high) / 2;
            if (wouldOverlap(route.pointAt(mid))) high = mid;
            else low = mid;
        }
        nextProgress = low;
        nextPoint = route.pointAt(nextProgress);
    }
    vehicle.speed = std::max(0.0, (nextProgress - vehicle.progress) / FIXED_STEP);
    vehicle.progress = nextProgress;
    vehicle.x = nextPoint.x;
    vehicle.y = nextPoint.y;
    vehicle.angle = nextPoint.angle;
    if (!vehicle.committed && permitted && vehicle.progress > stop + 1e-6) vehicle.committed = true;
    if (vehicle.speed < 3 && vehicle.progress > 45) vehicle.wait += FIXED_STEP;
}
void TrafficSimulation::fixedStep() {
    ++ticks_;
    time = static_cast<double>(ticks_)*FIXED_STEP;
    advanceSignals();
    spawnArrivals();
    std::vector<std::size_t> ordered(vehicles.size());
    std::iota(ordered.begin(), ordered.end(), 0);
    std::sort(ordered.begin(), ordered.end(), [&](std::size_t a, std::size_t b) {
        return vehicles[a].progress != vehicles[b].progress ? vehicles[a].progress > vehicles[b].progress : vehicles[a].id < vehicles[b].id;
    });
    for (std::size_t index : ordered) moveVehicle(vehicles[index]);
    vehicles.erase(std::remove_if(vehicles.begin(), vehicles.end(), [&](const Vehicle& vehicle) {
        if (vehicle.progress < routes()[vehicle.routeIndex].length - 1e-6) return false;
        ++completed_;
        completedWait_ += vehicle.wait;
        completedTimes_.push_back(time);
        return true;
    }), vehicles.end());
    while (!completedTimes_.empty() && completedTimes_.front() < time - 60) completedTimes_.pop_front();
    refreshState();
    const int second = static_cast<int>(std::floor(time));
    if (second != lastHistorySecond_) {
        lastHistorySecond_ = second;
        recentHistory.push_back({time, stats});
        if (recentHistory.size() > 120) recentHistory.pop_front();
    }
}

} // namespace smartflow
