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

// ---- Polygons ----------------------------------------------------------

// A point in continuous coordinates.
struct Point {
    double x;
    double y;
};

// A polygon is its vertices in order; the last vertex joins back to the first.
using Polygon = std::vector<Point>;

// Geometric transformations. Rotation and scaling are about `pivot`.
// Angles are in radians; with y pointing down, a positive angle turns clockwise on screen.
[[nodiscard]] Polygon translate(const Polygon& poly, double dx, double dy);
[[nodiscard]] Polygon rotate(const Polygon& poly, double angle, Point pivot);
[[nodiscard]] Polygon scale(const Polygon& poly, double sx, double sy, Point pivot);

// Shoelace formula. The sign gives the vertex order; the magnitude is the area.
[[nodiscard]] double signedArea(const Polygon& poly);

// Average of the vertices; a simple centre used as a rotation pivot.
[[nodiscard]] Point vertexCentre(const Polygon& poly);

// Outline pixels: a Bresenham line along every edge, including the closing edge.
[[nodiscard]] std::vector<Pixel> polygonOutline(const Polygon& poly);

} // namespace smartflow::graphics
