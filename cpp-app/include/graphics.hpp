#pragma once

#include <optional>
#include <vector>

namespace smartflow::graphics {

// These algorithms calculate coordinates. The graphics library only paints them.
struct Pixel {
    int x;
    int y;
};

struct ClipRect {
    double xmin;
    double ymin;
    double xmax;
    double ymax;
};

struct Segment {
    double x0;
    double y0;
    double x1;
    double y1;
};

// Includes both endpoints; works in every octant, including a single-point line.
std::vector<Pixel> bresenhamLine(int x0, int y0, int x1, int y1);

// Produces unique pixels in the original eight-way symmetry insertion order.
// Negative radii are treated as zero. Throws if a result cannot fit in an int.
std::vector<Pixel> midpointCircle(int cx, int cy, int radius);

// Inclusive rectangle boundaries; an invisible segment returns std::nullopt.
// Arguments must be finite, and the rectangle bounds must be ordered.
std::optional<Segment> clipLine(double x0, double y0, double x1, double y1,
                              const ClipRect& box);

} // namespace smartflow::graphics
