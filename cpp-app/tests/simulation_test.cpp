#include "simulation.hpp"

#include <cmath>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>

using namespace smartflow;

namespace {
void require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}
void close(double value, double expected, double tolerance = 1e-8) {
    require(std::abs(value - expected) <= tolerance, "Expected " + std::to_string(expected) + ", got " + std::to_string(value));
}
std::string snapshot(const TrafficSimulation& simulation) {
    std::ostringstream out;
    out << std::setprecision(17) << simulation.time << ' ' << simulation.phase << ' ' << simulation.phaseApproach << ' ' << simulation.phaseRemaining;
    const auto& stats = simulation.stats;
    out << ' ' << stats.active << ' ' << stats.queued << ' ' << stats.completed << ' ' << stats.averageWait << ' ' << stats.throughput << ' ' << stats.pending << ' ' << stats.lastMinuteThroughput;
    for (const auto& v : simulation.vehicles) {
        out << '\n' << v.id << ' ' << v.routeIndex << ' ' << v.approach << ' ' << v.turn << ' ' << v.destination
            << ' ' << v.progress << ' ' << v.speed << ' ' << v.wait << ' ' << v.committed << ' ' << v.x << ' ' << v.y << ' ' << v.angle;
    }
    return out.str();
}
void geometry() {
    require(routes().size() == 12, "Twelve routes must exist");
    std::set<std::string> ids;
    const std::map<char, std::pair<char, double>> incoming{{'N', {'x', 540}}, {'E', {'y', 440}}, {'S', {'x', 460}}, {'W', {'y', 360}}};
    const std::map<char, std::pair<char, double>> outgoing{{'N', {'x', 460}}, {'E', {'y', 360}}, {'S', {'x', 540}}, {'W', {'y', 440}}};
    const std::map<char, std::array<char, 3>> destinations{{'N', {'S','E','W'}}, {'E', {'W','S','N'}}, {'S', {'N','W','E'}}, {'W', {'E','N','S'}}};
    for (const auto& route : routes()) {
        ids.insert(route.id);
        const int turnIndex = route.turn == "straight" ? 0 : route.turn == "left" ? 1 : 2;
        require(route.destination == destinations.at(route.approach)[turnIndex], route.id + " has wrong destination");
        const Pose start = route.pointAt(0), end = route.pointAt(route.length);
        const auto entry = incoming.at(route.approach), exit = outgoing.at(route.destination);
        close(entry.first == 'x' ? start.x : start.y, entry.second);
        close(exit.first == 'x' ? end.x : end.y, exit.second);
        close(route.pointAt(-10).x, start.x);
        close(route.pointAt(route.length + 10).y, end.y);
        require(route.samples.size() == 103, "Route requires start, entry, 100 curve samples and end");
        require(route.stopProgress < route.entryProgress, "Stop line must precede curve entry");
        require(route.clearanceProgress < route.length, "Route must extend beyond junction clearance");
        Pose previous = start;
        for (double s = 2; s <= route.length; s += 2) {
            const Pose point = route.pointAt(s);
            require(std::isfinite(point.x) && std::isfinite(point.y) && std::isfinite(point.angle), "Geometry must remain finite");
            require(std::hypot(point.x - previous.x, point.y - previous.y) <= 2.001, route.id + " has a position jump");
            require(std::abs(std::fmod(point.angle - previous.angle + 540, 360) - 180) < 4, route.id + " has an abrupt heading change");
            previous = point;
        }
    }
    require(ids.size() == 12, "Route identifiers must be unique");
}
void deterministicReset() {
    TrafficSimulation simulation(2026, 35, 8);
    simulation.step(90);
    const auto first = snapshot(simulation);
    require(simulation.stats.completed > 0, "Scenario should complete trips");
    simulation.reset();
    require(simulation.time == 0 && simulation.vehicles.empty() && simulation.stats.completed == 0, "Reset must clear state");
    simulation.step(90);
    require(snapshot(simulation) == first, "Same seed and reset must reproduce state");
    TrafficSimulation other(3, 35, 8);
    other.step(90);
    require(snapshot(other) != first, "A different seed should change traffic");
}
void javascriptParity() {
    // Golden result was obtained from the preserved JavaScript model, seed 2026,
    // demand 35/min, eight-second greens, after exactly 90 simulation seconds.
    TrafficSimulation simulation(2026, 35, 8);
    simulation.step(90);
    require(simulation.phase == "allRed" && simulation.phaseApproach == 'S', "Port must retain signal schedule");
    close(simulation.phaseRemaining, 1);
    require(simulation.stats.active == 24 && simulation.stats.queued == 16 && simulation.stats.completed == 26 && simulation.stats.pending == 0, "Port must retain JS traffic totals");
    require(simulation.stats.lastMinuteThroughput == 23, "Port must retain rolling completions");
    close(simulation.stats.averageWait, 21.832692307692046);
    const auto& front = simulation.vehicles.front();
    require(front.id == 26 && routes()[front.routeIndex].id == "W-left", "Port must retain seeded route sequence");
    close(front.progress, 410);
    close(front.wait, 36.716666666665866);
    const auto& second = simulation.vehicles[1];
    require(second.id == 27 && routes()[second.routeIndex].id == "S-right", "Second seeded vehicle differs");
    close(second.progress, 880.0506069700069, 1e-5);
    close(second.x, 904.4005282146455, 1e-5);
}
void framePartition() {
    TrafficSimulation single, fast, irregular;
    single.step(30);
    for (int i = 0; i < 3600; ++i) fast.step(1.0 / 120);
    for (int i = 0; i < 300; ++i) { irregular.step(0.013); irregular.step(0.041); irregular.step(0.046); }
    require(snapshot(fast) == snapshot(single), "120 Hz updates must equal one duration");
    require(snapshot(irregular) == snapshot(single), "Irregular rendering frames must preserve model state");
}
void manualAdvance() {
    TrafficSimulation simulation(42, 4);
    require(simulation.signal('N') == "green", "North starts green");
    simulation.requestNextPhase();
    require(simulation.signal('N') == "amber", "Manual advance starts amber");
    close(simulation.phaseRemaining, 3);
    simulation.step(2.9);
    simulation.requestNextPhase();
    require(simulation.phase == "amber", "Repeated request must not bypass amber");
    simulation.step(0.1);
    require(simulation.phase == "allRed", "Amber lasts three seconds");
    for (char a : APPROACHES) require(simulation.signal(a) == "red", "All directions must show red");
    simulation.requestNextPhase();
    simulation.step(1.9);
    require(simulation.phase == "allRed", "Repeated request must not bypass clearance");
    simulation.step(0.1);
    require(simulation.signal('E') == "green" && simulation.signal('N') == "red", "East follows north after clearance");
}
void durationLatching() {
    TrafficSimulation simulation(42, 4, 10);
    simulation.step(4);
    simulation.setGreenDuration(6);
    close(simulation.phaseRemaining, 6);
    simulation.step(5.9);
    require(simulation.phase == "green", "New setting must not shorten current green");
    simulation.step(0.1);
    require(simulation.phase == "amber", "Original green duration must finish");
    simulation.step(5);
    require(simulation.phaseApproach == 'E' && simulation.phase == "green", "East phase should begin");
    close(simulation.phaseRemaining, 6);
    simulation.setGreenDuration(30);
    simulation.step(6);
    require(simulation.phase == "amber", "New setting must not extend active green");
    simulation.reset();
    close(simulation.phaseRemaining, 30);
}
void denseSafety() {
    TrafficSimulation simulation(42, 60, 6);
    std::set<int> seen;
    std::set<std::size_t> usedRoutes;
    std::map<char, int> entered;
    std::map<int, std::pair<double, bool>> previous;
    bool observedQueue = false, observedExtendedClearance = false;
    const double minimumDistance = std::hypot(VEHICLE_LENGTH, VEHICLE_WIDTH);
    for (int tick = 0; tick < 18000; ++tick) {
        simulation.step(FIXED_STEP);
        int greens = 0;
        for (char a : APPROACHES) if (simulation.signal(a) == "green") ++greens;
        require(greens <= 1, "Conflicting greens must never occur");
        observedQueue |= simulation.stats.queued > 2;
        observedExtendedClearance |= simulation.phase == "allRed" && simulation.phaseRemaining == 0 && simulation.intersectionOccupied();
        std::map<int, std::pair<double, bool>> current;
        for (const auto& vehicle : simulation.vehicles) {
            const auto& route = routes()[vehicle.routeIndex];
            seen.insert(vehicle.id);
            usedRoutes.insert(vehicle.routeIndex);
            require(std::isfinite(vehicle.x) && std::isfinite(vehicle.y) && std::isfinite(vehicle.angle) && std::isfinite(vehicle.progress) && std::isfinite(vehicle.speed), "Vehicle state must remain finite");
            require(vehicle.speed >= 0 && vehicle.speed <= 86.00001, "Speed must stay within bounds");
            const auto prior = previous.find(vehicle.id);
            if (prior != previous.end()) require(vehicle.progress >= prior->second.first, "Vehicles must never reverse to resolve overlap");
            if (!vehicle.committed) require(vehicle.progress <= route.stopProgress - vehicle.length / 2 - 4 + 1e-6, "Uncommitted car must stay behind stop position");
            if (vehicle.committed && (prior == previous.end() || !prior->second.second)) {
                require(simulation.signal(vehicle.approach) == "green", "New entry needs green");
                ++entered[vehicle.approach];
            }
            if (vehicle.committed && vehicle.progress < route.clearanceProgress && simulation.phase == "green") {
                require(vehicle.approach == simulation.phaseApproach, "Conflicting green must wait for committed traffic to clear");
            }
            current[vehicle.id] = {vehicle.progress, vehicle.committed};
        }
        for (std::size_t i = 0; i < simulation.vehicles.size(); ++i) {
            for (std::size_t j = i + 1; j < simulation.vehicles.size(); ++j) {
                const auto& a = simulation.vehicles[i];
                const auto& b = simulation.vehicles[j];
                require(std::hypot(a.x - b.x, a.y - b.y) >= minimumDistance, "Vehicles overlap during dense mixed traffic");
            }
        }
        previous = std::move(current);
    }
    require(observedQueue, "High demand must exercise stopped queues");
    require(observedExtendedClearance, "Test must exercise extended all-red clearance");
    require(usedRoutes.size() == 12, "Stress test must exercise every route");
    for (char a : APPROACHES) require(entered[a] > 10, "Every approach must receive service");
    require(simulation.stats.completed > 100, "Safety must not deadlock junction");
    require(simulation.stats.averageWait > 0 && simulation.stats.pending > 0, "Stress test must exercise wait accounting and offscreen demand");
    require(static_cast<int>(seen.size()) == simulation.stats.active + simulation.stats.completed, "Vehicles can leave only by completing routes");
    require(simulation.recentHistory.size() == 120, "History must remain bounded");
}
void boundedConfiguration() {
    TrafficSimulation simulation;
    require(simulation.seed == 42 && simulation.density == 24 && simulation.greenDuration == 10, "Defaults must match original");
    simulation.setDensity(1000);
    simulation.setGreenDuration(-20);
    require(simulation.density == 60 && simulation.greenDuration == 6, "Settings must clamp high/low inputs");
    simulation.setDensity(-1);
    simulation.setGreenDuration(100);
    const double nan = std::numeric_limits<double>::quiet_NaN();
    simulation.setDensity(nan);
    simulation.setGreenDuration(nan);
    simulation.step(nan);
    simulation.step(std::numeric_limits<double>::infinity());
    simulation.step(-1);
    require(simulation.time == 0 && simulation.density == 4 && simulation.greenDuration == 30, "Invalid inputs must not corrupt state");
    TrafficSimulation zeroDefaults(42, 0, 0);
    require(zeroDefaults.density == 24 && zeroDefaults.greenDuration == 10, "Constructor zero values use JS defaults");
}
} // namespace

int main() {
    const std::pair<const char*, std::function<void()>> tests[] = {
        {"twelve continuous left-driving routes", geometry},
        {"seeded reset reproduces state", deterministicReset},
        {"preserved JavaScript model golden result", javascriptParity},
        {"fixed updates are frame-rate independent", framePartition},
        {"manual signal advance preserves clearance", manualAdvance},
        {"green setting latches on next phase", durationLatching},
        {"300-second dense safety and fairness", denseSafety},
        {"bounded configuration and invalid time", boundedConfiguration},
    };
    int failed = 0;
    for (const auto& test : tests) {
        try { test.second(); std::cout << "PASS: " << test.first << '\n'; }
        catch (const std::exception& error) { ++failed; std::cerr << "FAIL: " << test.first << ": " << error.what() << '\n'; }
    }
    std::cout << (sizeof(tests) / sizeof(tests[0]) - failed) << "/" << sizeof(tests) / sizeof(tests[0]) << " tests passed\n";
    return failed ? 1 : 0;
}
