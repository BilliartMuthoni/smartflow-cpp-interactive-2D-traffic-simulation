#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <string>
#include <vector>

namespace smartflow {

inline constexpr std::array<char, 4> APPROACHES{{'N', 'E', 'S', 'W'}};
inline constexpr double FIXED_STEP = 1.0 / 60.0;
inline constexpr double VEHICLE_LENGTH = 32.0;
inline constexpr double VEHICLE_WIDTH = 17.0;
inline constexpr double MIN_GAP = 12.0;
inline constexpr double MAX_SPEED = 86.0;
inline constexpr double TURN_SPEED = 52.0;
inline constexpr double ACCELERATION = 28.0;
inline constexpr double BRAKING = 52.0;
inline constexpr double YELLOW_DURATION = 3.0;
inline constexpr double ALL_RED_DURATION = 2.0;
struct World { double width, height, cx, cy, roadHalfWidth; };
inline constexpr World WORLD{1000, 800, 500, 400, 90};

struct Vec2 { double x{}, y{}; };
struct Pose { double x{}, y{}, angle{}; };
struct Sample { double s{}, x{}, y{}; };
struct Route {
    std::string id;
    char approach{};
    std::string turn;
    char destination{};
    double length{}, entryProgress{}, exitProgress{}, clearanceProgress{}, stopProgress{};
    Vec2 start{}, end{};
    std::vector<Sample> samples;
    std::array<Vec2, 4> curve{};
    Pose pointAt(double progress) const;
};
const std::vector<Route>& routes();

struct Vehicle {
    int id{};
    std::size_t routeIndex{};
    char approach{};
    std::string turn;
    char destination{};
    double progress{}, speed{}, wait{}, x{}, y{}, angle{};
    bool committed{};
    int colorIndex{};
    double length{VEHICLE_LENGTH}, width{VEHICLE_WIDTH}, bornAt{};
};
struct Statistics {
    int active{}, queued{}, completed{}, pending{};
    double averageWait{}, throughput{};
    int lastMinuteThroughput{};
};
struct HistoryEntry {
    double time{};
    Statistics stats;
};

// The model uses world units and seconds; it has no graphics-library dependency.
// Public state is exposed for drawing and inspection. Change settings via setters.
class TrafficSimulation {
public:
    explicit TrafficSimulation(std::uint32_t seed = 42, double density = 24, double greenDuration = 10);
    void reset();
    void step(double dt);
    void setDensity(double number);
    void setGreenDuration(double number);
    void requestNextPhase();
    std::string signal(char approach) const;
    bool intersectionOccupied() const;
    std::string phaseLabel() const;

    std::uint32_t seed;
    std::vector<Vehicle> vehicles;
    Statistics stats;
    std::deque<HistoryEntry> recentHistory;
    double density, greenDuration, time{}, phaseRemaining{};
    std::string phase;
    char phaseApproach{};

private:
    struct Arrival { char approach; std::string turn; };
    std::uint32_t randomState_{};
    std::uint64_t ticks_{};
    double accumulator_{}, activeGreenDuration_{}, phaseElapsed_{};
    int nextId_{}, completed_{}, lastHistorySecond_{};
    double completedWait_{}, arrivalClock_{}, arrivalDelay_{};
    std::deque<double> completedTimes_;
    std::vector<Arrival> pending_;

    double random();
    double nextArrivalDelay();
    void refreshState();
    void advanceSignals();
    void spawnArrivals();
    double leaderGap(const Vehicle& vehicle, const Vehicle& other) const;
    void moveVehicle(Vehicle& vehicle);
    void fixedStep();
};

} // namespace smartflow
