#pragma once

#include <optional>
#include <vector>

namespace smartflow::graphics {

// One cell on the integer pixel grid.
struct Pixel {
    int x;
    int y;
};

// Axis-aligned clipping window; boundaries are inclusive.
struct ClipRect {
    double xmin;
    double ymin;
    double xmax;
    double ymax;
};

// A line segment in continuous (non-pixel) coordinates.
struct Segment {
    double x0;
    double y0;
    double x1;
    double y1;
};

// Bresenham's line algorithm using integer arithmetic only.
// Returns every pixel from (x0, y0) to (x1, y1), both endpoints included.
// Works in all eight octants; identical endpoints give a single pixel.
[[nodiscard]] std::vector<Pixel> bresenhamLine(int x0, int y0, int x1, int y1);

// Midpoint circle algorithm: computes one octant and mirrors it eight ways.
// Produces unique pixels in the original eight-way symmetry insertion order.
// Negative radii are treated as zero. Throws if a result cannot fit in an int.
[[nodiscard]] std::vector<Pixel> midpointCircle(int cx, int cy, int radius);

// Cohen–Sutherland line clipping against an inclusive rectangle.
// Returns the visible part of the segment, or std::nullopt if none is visible.
// Throws std::invalid_argument if any value is NaN/infinite, and the rectangle bounds must be ordered.
[[nodiscard]] std::optional<Segment> clipLine(double x0, double y0,
                                              double x1, double y1,
                                              const ClipRect& box);

} // namespace smartflow::graphics
