#include "graphics.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <unordered_set>

namespace smartflow::graphics {
namespace {

int checkedCoordinate(std::int64_t value) {
    if (value < std::numeric_limits<int>::min() ||
        value > std::numeric_limits<int>::max()) {
        throw std::overflow_error("Circle coordinate is outside the integer range.");
    }
    return static_cast<int>(value);
}

std::uint64_t pixelKey(int x, int y) {
    return (static_cast<std::uint64_t>(static_cast<std::uint32_t>(x)) << 32U) |
           static_cast<std::uint32_t>(y);
}

} // namespace

std::vector<Pixel> bresenhamLine(int x0, int y0, int x1, int y1) {
    std::vector<Pixel> points;
    // Widen before subtraction so even nearby INT_MIN/INT_MAX inputs are safe.
    const auto dx = std::abs(static_cast<std::int64_t>(x1) - x0);
    const int sx = x0 < x1 ? 1 : -1;
    const auto dy = -std::abs(static_cast<std::int64_t>(y1) - y0);
    const int sy = y0 < y1 ? 1 : -1;
    auto error = dx + dy;

    while (true) {
        points.push_back({x0, y0});
        if (x0 == x1 && y0 == y1) return points;
        const auto twice = 2 * error;
        if (twice >= dy) {
            error += dy;
            x0 += sx;
        }
        if (twice <= dx) {
            error += dx;
            y0 += sy;
        }
    }
}

std::vector<Pixel> midpointCircle(int cx, int cy, int radius) {
    std::int64_t x = std::max(0, radius);
    std::int64_t y = 0;
    auto decision = 1 - x;
    std::vector<Pixel> points;
    std::unordered_set<std::uint64_t> seen;

    while (x >= y) {
        const std::array<std::array<std::int64_t, 2>, 8> octants{{
            {x, y}, {y, x}, {-y, x}, {-x, y},
            {-x, -y}, {-y, -x}, {y, -x}, {x, -y}
        }};
        for (const auto& offset : octants) {
            const int px = checkedCoordinate(static_cast<std::int64_t>(cx) + offset[0]);
            const int py = checkedCoordinate(static_cast<std::int64_t>(cy) + offset[1]);
            if (seen.insert(pixelKey(px, py)).second) points.push_back({px, py});
        }
        ++y;
        if (decision < 0) {
            decision += 2 * y + 1;
        } else {
            --x;
            decision += 2 * (y - x) + 1;
        }
    }
    return points;
}

std::optional<Segment> clipLine(double x0, double y0, double x1, double y1,
                              const ClipRect& box) {
    const std::array<double, 8> inputs{
        x0, y0, x1, y1, box.xmin, box.ymin, box.xmax, box.ymax
    };
    if (!std::all_of(inputs.begin(), inputs.end(), [](double v) { return std::isfinite(v); }) ||
        box.xmin > box.xmax || box.ymin > box.ymax) {
        throw std::invalid_argument("Clipping requires finite coordinates and ordered bounds.");
    }

    const auto code = [&box](double x, double y) {
        return (x < box.xmin ? 1 : x > box.xmax ? 2 : 0) |
               (y < box.ymin ? 4 : y > box.ymax ? 8 : 0);
    };
    int a = code(x0, y0);
    int b = code(x1, y1);
    for (int iteration = 0; iteration < 16; ++iteration) {
        if (!(a | b)) return Segment{x0, y0, x1, y1};
        if (a & b) return std::nullopt;
        const int outside = a ? a : b;
        double x;
        double y;
        if (outside & 8) {
            x = x0 + (x1 - x0) * (box.ymax - y0) / (y1 - y0);
            y = box.ymax;
        } else if (outside & 4) {
            x = x0 + (x1 - x0) * (box.ymin - y0) / (y1 - y0);
            y = box.ymin;
        } else if (outside & 2) {
            y = y0 + (y1 - y0) * (box.xmax - x0) / (x1 - x0);
            x = box.xmax;
        } else {
            y = y0 + (y1 - y0) * (box.xmin - x0) / (x1 - x0);
            x = box.xmin;
        }
        if (outside == a) {
            x0 = x;
            y0 = y;
            a = code(x0, y0);
        } else {
            x1 = x;
            y1 = y;
            b = code(x1, y1);
        }
    }
    throw std::runtime_error("Line clipping did not converge.");
}

} // namespace smartflow::graphics
