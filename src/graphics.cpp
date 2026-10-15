#include "graphics.hpp"

#include <algorithm>  // std::max
#include <cmath>      // std::isfinite
#include <cstdlib>    // std::abs
#include <set>
#include <stdexcept>
#include <utility>    // std::pair

namespace smartflow::graphics {

std::vector<Pixel> bresenhamLine(int x0, int y0, int x1, int y1) {
    std::vector<Pixel> points;
    int dx = std::abs(x1 - x0);
    int dy = -std::abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx + dy;

    while (true) {
        points.push_back({x0, y0});
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
    return points;
}

std::vector<Pixel> midpointCircle(int cx, int cy, int radius) {
    std::vector<Pixel> points;
    std::set<std::pair<int, int>> seen;

    auto plot = [&](int px, int py) {
        if (seen.insert({px, py}).second) {
            points.push_back({px, py});
        }
    };

    int x = std::max(0, radius);
    int y = 0;
    int p = 1 - x;

    while (x >= y) {
        plot(cx + x, cy + y);  plot(cx + y, cy + x);
        plot(cx - y, cy + x);  plot(cx - x, cy + y);
        plot(cx - x, cy - y);  plot(cx - y, cy - x);
        plot(cx + y, cy - x);  plot(cx + x, cy - y);

        ++y;
        if (p < 0) {
            p += 2 * y + 1;
        } else {
            --x;
            p += 2 * (y - x) + 1;
        }
    }
    return points;

}

// Region codes: one bit per side of the window the point lies beyond.
namespace {
constexpr int INSIDE = 0, LEFT = 1, RIGHT = 2, BOTTOM = 4, TOP = 8;

int regionCode(double x, double y, const ClipRect& box) {
    int code = INSIDE;
    if (x < box.xmin) code |= LEFT;
    else if (x > box.xmax) code |= RIGHT;
    if (y < box.ymin) code |= BOTTOM;
    else if (y > box.ymax) code |= TOP;
    return code;
}
} // namespace

std::optional<Segment> clipLine(double x0, double y0, double x1, double y1,
                                const ClipRect& box) {
    for (double v : {x0, y0, x1, y1, box.xmin, box.ymin, box.xmax, box.ymax}) {
        if (!std::isfinite(v)) throw std::invalid_argument("Clipping needs finite values.");
    }
    if (box.xmin > box.xmax || box.ymin > box.ymax) {
        throw std::invalid_argument("Clip window bounds must be ordered.");
    }

    int code0 = regionCode(x0, y0, box);
    int code1 = regionCode(x1, y1, box);

    // Each pass moves one outside endpoint onto a window edge, so at most
    // four passes per endpoint are needed; the limit guards against rounding loops.
    for (int pass = 0; pass < 16; ++pass) {
        if ((code0 | code1) == 0) return Segment{x0, y0, x1, y1};  // trivially accept
        if ((code0 & code1) != 0) return std::nullopt;             // trivially reject

        // Pick an endpoint that is outside and intersect with the edge it is beyond.
        const int out = code0 ? code0 : code1;
        double x = 0, y = 0;
        if (out & TOP) {
            x = x0 + (x1 - x0) * (box.ymax - y0) / (y1 - y0);
            y = box.ymax;
        } else if (out & BOTTOM) {
            x = x0 + (x1 - x0) * (box.ymin - y0) / (y1 - y0);
            y = box.ymin;
        } else if (out & RIGHT) {
            y = y0 + (y1 - y0) * (box.xmax - x0) / (x1 - x0);
            x = box.xmax;
        } else {
            y = y0 + (y1 - y0) * (box.xmin - x0) / (x1 - x0);
            x = box.xmin;
        }

        if (out == code0) { x0 = x; y0 = y; code0 = regionCode(x0, y0, box); }
        else              { x1 = x; y1 = y; code1 = regionCode(x1, y1, box); }
    }
    throw std::runtime_error("Line clipping did not converge.");
}

} // namespace smartflow::graphics
