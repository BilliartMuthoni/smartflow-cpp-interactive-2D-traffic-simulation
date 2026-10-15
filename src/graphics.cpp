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

// ---- Polygons ----------------------------------------------------------

Polygon translate(const Polygon& poly, double dx, double dy) {
    Polygon out;
    for (const auto& p : poly) out.push_back({p.x + dx, p.y + dy});
    return out;
}

// Rotation about a pivot: move the pivot to the origin, apply
//   x' = x cos(a) - y sin(a),   y' = x sin(a) + y cos(a),
// then move back.
Polygon rotate(const Polygon& poly, double angle, Point pivot) {
    const double c = std::cos(angle);
    const double s = std::sin(angle);
    Polygon out;
    for (const auto& p : poly) {
        const double x = p.x - pivot.x;
        const double y = p.y - pivot.y;
        out.push_back({pivot.x + x * c - y * s, pivot.y + x * s + y * c});
    }
    return out;
}

Polygon scale(const Polygon& poly, double sx, double sy, Point pivot) {
    Polygon out;
    for (const auto& p : poly) {
        out.push_back({pivot.x + (p.x - pivot.x) * sx, pivot.y + (p.y - pivot.y) * sy});
    }
    return out;
}

// Shoelace: area = 1/2 * sum(x_i * y_{i+1} - x_{i+1} * y_i).
double signedArea(const Polygon& poly) {
    double sum = 0;
    for (std::size_t i = 0; i < poly.size(); ++i) {
        const Point& a = poly[i];
        const Point& b = poly[(i + 1) % poly.size()];
        sum += a.x * b.y - b.x * a.y;
    }
    return sum / 2;
}

Point vertexCentre(const Polygon& poly) {
    Point c{0, 0};
    if (poly.empty()) return c;
    for (const auto& p : poly) { c.x += p.x; c.y += p.y; }
    return {c.x / poly.size(), c.y / poly.size()};
}

std::vector<Pixel> polygonOutline(const Polygon& poly) {
    std::vector<Pixel> pixels;
    for (std::size_t i = 0; i < poly.size(); ++i) {
        const Point& a = poly[i];
        const Point& b = poly[(i + 1) % poly.size()];
        const auto edge = bresenhamLine(int(std::lround(a.x)), int(std::lround(a.y)),
                                        int(std::lround(b.x)), int(std::lround(b.y)));
        pixels.insert(pixels.end(), edge.begin(), edge.end());
    }
    return pixels;
}

std::vector<Pixel> scanlineFill(const Polygon& poly) {
    std::vector<Pixel> pixels;
    if (poly.size() < 3) return pixels;

    double minY = poly[0].y, maxY = poly[0].y;
    for (const auto& p : poly) { minY = std::min(minY, p.y); maxY = std::max(maxY, p.y); }

    // Pixel row y is sampled through its centre, y + 0.5.
    for (int y = int(std::floor(minY)); y <= int(std::ceil(maxY)); ++y) {
        const double scanY = y + 0.5;

        // 1. Find where this scan line crosses each edge. The half-open test
        //    (one end <= scanY, the other > scanY) counts a shared vertex once
        //    and skips horizontal edges.
        std::vector<double> crossings;
        for (std::size_t i = 0; i < poly.size(); ++i) {
            const Point& a = poly[i];
            const Point& b = poly[(i + 1) % poly.size()];
            if ((a.y <= scanY) != (b.y <= scanY)) {
                crossings.push_back(a.x + (scanY - a.y) * (b.x - a.x) / (b.y - a.y));
            }
        }

        // 2. Sort the crossings from left to right.
        std::sort(crossings.begin(), crossings.end());

        // 3. Fill between pairs: inside after the 1st crossing, outside after the 2nd, ...
        for (std::size_t i = 0; i + 1 < crossings.size(); i += 2) {
            const int xStart = int(std::ceil(crossings[i] - 0.5));
            const int xEnd = int(std::floor(crossings[i + 1] - 0.5));
            for (int x = xStart; x <= xEnd; ++x) pixels.push_back({x, y});
        }
    }
    return pixels;
}

} // namespace smartflow::graphics
