#include "graphics.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>

using namespace smartflow::graphics;

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

bool near(double a, double b) { return std::abs(a - b) < 1e-9; }

void expectSegment(const std::optional<Segment>& actual, const Segment& expected) {
    require(actual.has_value(), "Expected a visible segment.");
    require(near(actual->x0, expected.x0) && near(actual->y0, expected.y0) &&
            near(actual->x1, expected.x1) && near(actual->y1, expected.y1),
            "Clipped segment does not match expected endpoints.");
}

void testBresenham() {
    // All octants, both directions, steep lines, shallow lines, and point lines.
    for (int x0 = -4; x0 <= 4; ++x0) {
        for (int y0 = -4; y0 <= 4; ++y0) {
            for (int x1 = -4; x1 <= 4; ++x1) {
                for (int y1 = -4; y1 <= 4; ++y1) {
                    const auto points = bresenhamLine(x0, y0, x1, y1);
                    const int dx = x1 - x0;
                    const int dy = y1 - y0;
                    const int major = std::max(std::abs(dx), std::abs(dy));
                    require(points.size() == static_cast<std::size_t>(major + 1), "Wrong line length.");
                    require(points.front().x == x0 && points.front().y == y0, "Missing start pixel.");
                    require(points.back().x == x1 && points.back().y == y1, "Missing end pixel.");
                    for (std::size_t i = 0; i < points.size(); ++i) {
                        const auto& p = points[i];
                        require(p.x >= std::min(x0, x1) && p.x <= std::max(x0, x1) &&
                                p.y >= std::min(y0, y1) && p.y <= std::max(y0, y1), "Pixel outside endpoint bounds.");
                        // Perpendicular error expressed in major-axis units is <= 1/2 pixel.
                        require(2 * std::abs(dy * (p.x - x0) - dx * (p.y - y0)) <= major,
                                "Line pixel is too far from the ideal segment.");
                        if (i > 0) {
                            const int stepx = std::abs(p.x - points[i - 1].x);
                            const int stepy = std::abs(p.y - points[i - 1].y);
                            require(std::max(stepx, stepy) == 1, "Disconnected or duplicate line pixel.");
                        }
                    }
                }
            }
        }
    }
    const auto tie = bresenhamLine(0, 0, 4, 2);
    require(tie[1].x == 1 && tie[1].y == 1 && tie[3].x == 3 && tie[3].y == 2,
            "Tie rule differs from the original implementation.");
    const auto high = bresenhamLine(std::numeric_limits<int>::max() - 2, 0,
                                   std::numeric_limits<int>::max(), 2);
    const auto low = bresenhamLine(std::numeric_limits<int>::min() + 2, 0,
                                  std::numeric_limits<int>::min(), -2);
    require(high.size() == 3 && low.size() == 3, "Near-limit coordinates failed.");
}

void testCircle() {
    for (const int radius : {-3, 0, 1, 2, 3, 7, 24, 100}) {
        constexpr int cx = -17;
        constexpr int cy = 31;
        const auto points = midpointCircle(cx, cy, radius);
        std::set<std::pair<int, int>> unique;
        for (const auto& p : points) unique.emplace(p.x - cx, p.y - cy);
        require(points.size() == unique.size(), "Circle contains duplicate pixels.");
        const int r = std::max(0, radius);
        require(points.front().x == cx + r && points.front().y == cy, "Changed symmetry insertion order.");
        require(unique.count({r, 0}) && unique.count({-r, 0}) &&
                unique.count({0, r}) && unique.count({0, -r}), "Missing circle cardinal point.");
        for (const auto& offset : unique) {
            const int x = offset.first;
            const int y = offset.second;
            const std::array<std::pair<int, int>, 8> symmetric{{
                {x, y}, {y, x}, {-y, x}, {-x, y}, {-x, -y}, {-y, -x}, {y, -x}, {x, -y}
            }};
            for (const auto& match : symmetric) require(unique.count(match) == 1, "Broken circle symmetry.");
            require(std::abs(std::hypot(x, y) - r) <= 0.5, "Circle sample too far from ideal radius.");
        }
        if (r == 0) require(points.size() == 1, "Zero-radius circle should be the center.");
        if (r == 1) require(points.size() == 4, "Unit circle should have four unique pixels.");
        if (r == 2) require(points.size() == 12, "Radius-two circle should have twelve unique pixels.");
    }
    bool overflowCaught = false;
    try { midpointCircle(std::numeric_limits<int>::max(), 0, 1); }
    catch (const std::overflow_error&) { overflowCaught = true; }
    require(overflowCaught, "Out-of-range circle coordinates should be rejected.");
}

void testClipping() {
    const ClipRect box{0, 0, 10, 10};
    expectSegment(clipLine(2, 3, 8, 9, box), {2, 3, 8, 9});
    expectSegment(clipLine(-5, 5, 15, 5, box), {0, 5, 10, 5});
    expectSegment(clipLine(5, 15, 5, -5, box), {5, 10, 5, 0});
    expectSegment(clipLine(-5, -5, 15, 15, box), {0, 0, 10, 10});
    expectSegment(clipLine(-5, 15, 15, -5, box), {0, 10, 10, 0});
    expectSegment(clipLine(-5, 0, 15, 0, box), {0, 0, 10, 0});
    expectSegment(clipLine(-5, 5, 5, 15, box), {0, 10, 0, 10});
    expectSegment(clipLine(0, 0, 0, 0, box), {0, 0, 0, 0});
    expectSegment(clipLine(5, 5, 5, 5, box), {5, 5, 5, 5});
    require(!clipLine(-1, 5, -1, 5, box), "Outside point should be invisible.");
    require(!clipLine(-5, -1, 15, -1, box), "Parallel outside segment should be invisible.");
    require(!clipLine(-5, 6, 6, 15, box), "Corner-missing segment should be invisible.");
    require(!clipLine(11, 1, 11, 9, box), "Right-side segment should be invisible.");
    expectSegment(clipLine(-5, 5, 15, 5, {5, 0, 5, 10}), {5, 5, 5, 5});
    expectSegment(clipLine(0, 0, 10, 10, {5, 5, 5, 5}), {5, 5, 5, 5});

    // Check bounds, collinearity, and endpoint reversal over a grid of segments.
    for (int x0 = -5; x0 <= 15; x0 += 5) {
        for (int y0 = -5; y0 <= 15; y0 += 5) {
            for (int x1 = -5; x1 <= 15; x1 += 5) {
                for (int y1 = -5; y1 <= 15; y1 += 5) {
                    const auto result = clipLine(x0, y0, x1, y1, box);
                    const auto reversed = clipLine(x1, y1, x0, y0, box);
                    require(result.has_value() == reversed.has_value(), "Clipping direction changed visibility.");
                    if (!result) continue;
                    expectSegment(reversed, {result->x1, result->y1, result->x0, result->y0});
                    for (const auto& p : std::array<std::pair<double, double>, 2>{{
                        {result->x0, result->y0}, {result->x1, result->y1}}}) {
                        require(p.first >= 0 && p.first <= 10 && p.second >= 0 && p.second <= 10,
                                "Clipped endpoint outside viewport.");
                        require(near((x1 - x0) * (p.second - y0), (y1 - y0) * (p.first - x0)),
                                "Clipped endpoint left the original line.");
                    }
                }
            }
        }
    }
    bool badBoundsCaught = false;
    try { clipLine(0, 0, 1, 1, {10, 0, 0, 10}); }
    catch (const std::invalid_argument&) { badBoundsCaught = true; }
    require(badBoundsCaught, "Reversed rectangle bounds should be rejected.");
    bool nonfiniteCaught = false;
    try { clipLine(std::numeric_limits<double>::infinity(), 0, 1, 1, box); }
    catch (const std::invalid_argument&) { nonfiniteCaught = true; }
    require(nonfiniteCaught, "Non-finite coordinates should be rejected.");
}

} // namespace

int main() {
    try {
        testBresenham();
        testCircle();
        testClipping();
        std::cout << "PASS: Bresenham (all octants), midpoint circle, Cohen-Sutherland clipping.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
