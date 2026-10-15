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

// Scan-line fill (even-odd rule). For each pixel row, finds where the row
// crosses the polygon's edges, sorts those crossings, and fills between pairs.
// Works for concave polygons. A pixel is filled when its centre is inside.
[[nodiscard]] std::vector<Pixel> scanlineFill(const Polygon& poly);

// Sutherland–Hodgman polygon clipping against a rectangular window.
// The polygon is clipped against each window edge in turn; the result is the
// part of the polygon inside the window (empty if none of it is visible).
[[nodiscard]] Polygon clipPolygon(const Polygon& poly, const ClipRect& box);

// ---- Ellipses ----------------------------------------------------------

// Midpoint ellipse algorithm with horizontal radius rx and vertical radius ry.
// Region 1 steps in x while the slope is shallow, region 2 steps in y once it
// is steep; each computed point is mirrored into all four quadrants.
// Produces unique pixels. Negative radii are treated as zero; a zero radius
// gives a straight line.
[[nodiscard]] std::vector<Pixel> midpointEllipse(int cx, int cy, int rx, int ry);

// ---- Curves ------------------------------------------------------------

// Cubic Bézier curve at parameter t in [0, 1]:
//   B(t) = (1-t)^3 P0 + 3(1-t)^2 t P1 + 3(1-t) t^2 P2 + t^3 P3
// The curve starts at P0, ends at P3, and is pulled towards P1 and P2.
[[nodiscard]] Point cubicBezier(Point p0, Point p1, Point p2, Point p3, double t);

// `segments + 1` evenly spaced samples of a cubic Bézier curve, from P0 to P3.
[[nodiscard]] std::vector<Point> bezierPoints(Point p0, Point p1, Point p2, Point p3, int segments);

// Uniform Catmull–Rom spline segment between P1 (t = 0) and P2 (t = 1);
// P0 and P3 set the tangents. Unlike Bézier, it passes through its control points.
[[nodiscard]] Point catmullRom(Point p0, Point p1, Point p2, Point p3, double t);

// A smooth curve through every control point, `segmentsPerSpan` samples per span.
// The first and last points are repeated so the curve reaches both ends.
[[nodiscard]] std::vector<Point> catmullRomSpline(const std::vector<Point>& control, int segmentsPerSpan);

// Pixels of a polyline: Bresenham lines between consecutive points.
[[nodiscard]] std::vector<Pixel> polylinePixels(const std::vector<Point>& points);

} // namespace smartflow::graphics
