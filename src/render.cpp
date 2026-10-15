// Renders the checkpoint images using only our own algorithms and the BMP canvas.
// Usage: render [output-folder]   (default: output)

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <string>

#include "canvas.hpp"
#include "graphics.hpp"

using namespace smartflow::graphics;

namespace {

constexpr int W = 400;      // canvas size in pixels
constexpr int H = 300;
constexpr int SCALE = 2;    // saved image is 800 x 600 so single pixels are visible

constexpr Color GRASS{214, 232, 200};
constexpr Color ASPHALT{70, 74, 80};
constexpr Color WHITE{250, 250, 250};
constexpr Color YELLOW{240, 200, 40};
constexpr Color BLUE{40, 90, 200};
constexpr Color GREEN{20, 160, 70};
constexpr Color GREY{185, 185, 185};
constexpr Color RED{220, 70, 60};

void line(Canvas& c, int x0, int y0, int x1, int y1, Color color) {
    c.plot(bresenhamLine(x0, y0, x1, y1), color);
}

// Dashed line: draws `dash` pixels, skips `gap` pixels, along the Bresenham path.
void dashedLine(Canvas& c, int x0, int y0, int x1, int y1, int dash, int gap, Color color) {
    const auto pts = bresenhamLine(x0, y0, x1, y1);
    for (std::size_t i = 0; i < pts.size(); ++i) {
        if (static_cast<int>(i % (dash + gap)) < dash) c.set(pts[i].x, pts[i].y, color);
    }
}

void rectOutline(Canvas& c, int x0, int y0, int x1, int y1, Color color) {
    line(c, x0, y0, x1, y0, color);
    line(c, x1, y0, x1, y1, color);
    line(c, x1, y1, x0, y1, color);
    line(c, x0, y1, x0, y0, color);
}

// Image 1 — Bresenham lines: junction road edges, lane markings and stop lines,
// plus a fan of lines in the corner showing all eight octants.
void renderLines(const std::string& dir) {
    Canvas c(W, H, GRASS);

    // Road edges: a horizontal road (y 120..180) crossing a vertical road (x 170..230).
    line(c, 0, 120, 170, 120, ASPHALT);   line(c, 230, 120, W - 1, 120, ASPHALT);
    line(c, 0, 180, 170, 180, ASPHALT);   line(c, 230, 180, W - 1, 180, ASPHALT);
    line(c, 170, 0, 170, 120, ASPHALT);   line(c, 170, 180, 170, H - 1, ASPHALT);
    line(c, 230, 0, 230, 120, ASPHALT);   line(c, 230, 180, 230, H - 1, ASPHALT);

    // Dashed centre lines, stopping before the junction box.
    dashedLine(c, 0, 150, 160, 150, 8, 6, YELLOW);
    dashedLine(c, 240, 150, W - 1, 150, 8, 6, YELLOW);
    dashedLine(c, 200, 0, 200, 110, 8, 6, YELLOW);
    dashedLine(c, 200, 190, 200, H - 1, 8, 6, YELLOW);

    // Stop lines on the approach side of each lane (left-hand traffic).
    line(c, 165, 150, 165, 180, RED);
    line(c, 235, 120, 235, 150, RED);
    line(c, 170, 115, 200, 115, RED);
    line(c, 200, 185, 230, 185, RED);

    // Octant fan: 16 lines from one centre, covering every slope and direction.
    const int cx = 70, cy = 60, r = 45;
    for (int k = 0; k < 16; ++k) {
        const double a = k * 3.14159265358979 / 8;
        line(c, cx, cy, cx + int(std::lround(r * std::cos(a))), cy + int(std::lround(r * std::sin(a))), BLUE);
    }

    c.saveBmp(dir + "/01-bresenham-lines.bmp", SCALE);
}

// Image 2 — Cohen–Sutherland: vehicle path segments clipped to the junction viewport.
// Grey = original segment, green = visible part kept, red = rejected.
void renderLineClipping(const std::string& dir) {
    Canvas c(W, H, WHITE);
    const ClipRect view{80, 60, 320, 240};

    const Segment paths[] = {
        {20, 150, 380, 150},   // straight through, cut on both sides
        {200, 10, 200, 290},   // vertical through
        {30, 280, 370, 20},    // diagonal across two corners
        {120, 100, 280, 200},  // fully inside: accepted unchanged
        {10, 20, 60, 290},     // left of the window: rejected
        {340, 30, 390, 280},   // right of the window: rejected
        {20, 100, 120, 10},    // passes the top-left corner but misses: rejected
        {250, 280, 390, 170},  // enters through the bottom-right corner area
    };

    for (const auto& p : paths) {
        const auto visible = clipLine(p.x0, p.y0, p.x1, p.y1, view);
        line(c, int(p.x0), int(p.y0), int(p.x1), int(p.y1), visible ? GREY : RED);
        if (visible) {
            line(c, int(std::lround(visible->x0)), int(std::lround(visible->y0)),
                 int(std::lround(visible->x1)), int(std::lround(visible->y1)), GREEN);
        }
    }
    rectOutline(c, int(view.xmin), int(view.ymin), int(view.xmax), int(view.ymax), BLUE);

    c.saveBmp(dir + "/02-cohen-sutherland-clipping.bmp", SCALE);
}

// Filled disc built from the midpoint circle: for each row the circle touches,
// join its leftmost and rightmost circle pixels with a horizontal span.
void disc(Canvas& c, int cx, int cy, int r, Color color) {
    const auto ring = midpointCircle(cx, cy, r);
    for (int y = cy - r; y <= cy + r; ++y) {
        int left = cx, right = cx;
        for (const auto& p : ring) {
            if (p.y == y) { left = std::min(left, p.x); right = std::max(right, p.x); }
        }
        line(c, left, y, right, y, color);
    }
}

// One signal head: a housing with red, amber and green lamps. `lit` is 0, 1 or 2.
void signalHead(Canvas& c, int x, int y, int lit) {
    constexpr Color HOUSING{40, 40, 45};
    constexpr Color LAMPS[] = {{225, 45, 40}, {245, 170, 30}, {30, 185, 80}};
    constexpr Color OFF{110, 110, 115};
    rectOutline(c, x, y, x + 24, y + 66, HOUSING);
    for (int i = 0; i < 3; ++i) {
        const int cy = y + 12 + i * 21;
        if (i == lit) disc(c, x + 12, cy, 8, LAMPS[i]);
        c.plot(midpointCircle(x + 12, cy, 8), i == lit ? HOUSING : OFF);
    }
}

// Image 3 — Midpoint circle: traffic-light lamps for the four junction approaches,
// plus concentric circles showing the algorithm at radii from 2 to 42.
void renderCircles(const std::string& dir) {
    Canvas c(W, H, WHITE);

    // Concentric circles: every radius produces a closed, symmetric ring.
    for (int r = 2; r <= 42; r += 5) c.plot(midpointCircle(70, 150, r), BLUE);

    // Four signal heads, one per approach, each showing a different phase.
    signalHead(c, 170, 40, 0);   // north: red
    signalHead(c, 230, 40, 1);   // east: amber
    signalHead(c, 290, 40, 2);   // south: green
    signalHead(c, 350, 40, 0);   // west: red

    // Large lamp so the stepped pixel pattern of the algorithm is easy to see.
    c.plot(midpointCircle(260, 210, 60), ASPHALT);
    c.plot(midpointCircle(260, 210, 20), RED);
    disc(c, 260, 210, 12, RED);

    c.saveBmp(dir + "/03-midpoint-circle.bmp", SCALE);
}

// A car seen from above, pointing along +x, centred on the origin.
const Polygon CAR{{-11, -6}, {7, -6}, {11, -3}, {11, 3}, {7, 6}, {-11, 6}};

// Places the car: rotate about its own centre, then move it to (x, y).
Polygon placeCar(double x, double y, double heading) {
    return translate(rotate(CAR, heading, {0, 0}), x, y);
}

// An L-shaped (concave) building footprint.
const Polygon BUILDING{{0, 0}, {50, 0}, {50, 20}, {25, 20}, {25, 45}, {0, 45}};

// Image 4 — Polygon representation and transformations: a car body
// translated and rotated through a left turn, and a building scaled.
void renderPolygons(const std::string& dir) {
    Canvas c(W, H, WHITE);
    const double pi = std::acos(-1.0);

    // Road edges for context.
    line(c, 0, 120, 170, 120, GREY);   line(c, 230, 120, W - 1, 120, GREY);
    line(c, 0, 180, 170, 180, GREY);   line(c, 230, 180, W - 1, 180, GREY);
    line(c, 170, 180, 170, H - 1, GREY);   line(c, 230, 180, 230, H - 1, GREY);
    line(c, 170, 0, 170, 120, GREY);   line(c, 230, 0, 230, 120, GREY);

    // The original car at the origin would be off-screen; show it at a reference spot.
    c.plot(polygonOutline(translate(CAR, 40, 60)), ASPHALT);

    // Left turn (left-hand traffic): eastbound in the upper lane, turning north.
    // Each copy is the same polygon, rotated to the path's heading and translated onto it.
    c.plot(polygonOutline(placeCar(80, 135, 0)), BLUE);
    for (int k = 0; k <= 4; ++k) {
        const double a = k * pi / 8;                       // 0 to 90 degrees along the arc
        const double x = 130 + 55 * std::sin(a);
        const double y = 60 + 75 * std::cos(a);
        const double heading = std::atan2(-75 * std::sin(a), 55 * std::cos(a));
        c.plot(polygonOutline(placeCar(x, y, heading)), BLUE);
    }

    // Scaling: the same footprint at 60%, 100% and 140% about its corner.
    for (double s : {0.6, 1.0, 1.4}) {
        c.plot(polygonOutline(translate(scale(BUILDING, s, s, {0, 0}), 260, 195)), GREEN);
    }
    // And a building rotated by 15 degrees about its centre.
    const Polygon placed = translate(BUILDING, 280, 30);
    c.plot(polygonOutline(rotate(placed, pi / 12, vertexCentre(placed))), RED);
    c.plot(polygonOutline(placed), GREY);

    c.saveBmp(dir + "/04-polygon-transformations.bmp", SCALE);
}

Polygon rectangle(double x0, double y0, double x1, double y1) {
    return {{x0, y0}, {x1, y0}, {x1, y1}, {x0, y1}};
}

void fill(Canvas& c, const Polygon& poly, Color color) { c.plot(scanlineFill(poly), color); }

constexpr Color ROOF_A{196, 120, 90};
constexpr Color ROOF_B{150, 130, 170};
constexpr Color CAR_COLORS[] = {{30, 110, 200}, {210, 60, 50}, {240, 180, 30}, {40, 150, 90}};

// Image 5 — Scan-line fill: road surfaces, concave building footprints and car bodies
// are filled polygons; outlines are drawn on top with Bresenham.
void renderFill(const std::string& dir) {
    Canvas c(W, H, GRASS);
    const double pi = std::acos(-1.0);

    // Road surfaces: a horizontal and a vertical rectangle (overlap at the junction).
    fill(c, rectangle(0, 120, W, 180), ASPHALT);
    fill(c, rectangle(170, 0, 230, H), ASPHALT);

    // Concave L-shaped buildings in two corners, filled then outlined.
    for (const auto& b : {translate(BUILDING, 40, 30), translate(scale(BUILDING, -1, 1, {0, 0}), 360, 30)}) {
        fill(c, b, ROOF_A);
        c.plot(polygonOutline(b), ASPHALT);
    }
    // A rotated rectangular building: the fill works for any orientation.
    const Polygon block = rectangle(270, 210, 350, 260);
    const Polygon tilted = rotate(block, pi / 10, vertexCentre(block));
    fill(c, tilted, ROOF_B);
    c.plot(polygonOutline(tilted), ASPHALT);

    // A concave star shape (a fountain plaza) to show the even-odd crossings.
    Polygon star;
    for (int k = 0; k < 10; ++k) {
        const double r = k % 2 ? 14 : 34;
        const double a = k * pi / 5 - pi / 2;
        star.push_back({90 + r * std::cos(a), 240 + r * std::sin(a)});
    }
    fill(c, star, YELLOW);
    c.plot(polygonOutline(star), ASPHALT);

    // Cars, each a filled polygon placed by rotation and translation.
    const struct { double x, y, heading; } cars[] = {
        {60, 135, 0}, {110, 135, 0}, {320, 165, pi}, {185, 70, -pi / 2}, {215, 240, pi / 2},
    };
    int i = 0;
    for (const auto& car : cars) {
        const Polygon body = placeCar(car.x, car.y, car.heading);
        fill(c, body, CAR_COLORS[i++ % 4]);
        c.plot(polygonOutline(body), WHITE);
    }

    c.saveBmp(dir + "/05-scanline-fill.bmp", SCALE);
}

constexpr Color PALE{232, 234, 238};

// Image 6 — Sutherland–Hodgman: the junction viewport (blue) clips every car and
// building that crosses it. Grey outline = whole polygon, filled = clipped result.
void renderPolygonClipping(const std::string& dir) {
    Canvas c(W, H, WHITE);
    const double pi = std::acos(-1.0);
    const ClipRect view{90, 60, 310, 240};

    const std::vector<Polygon> shapes = {
        placeCar(94, 135, 0),              // entering from the left edge
        placeCar(306, 165, pi),            // entering from the right edge
        placeCar(185, 64, -pi / 2),        // entering from the top edge
        placeCar(150, 150, -pi / 5),       // fully inside: unchanged
        placeCar(40, 220, pi / 4),         // fully outside: removed
        translate(BUILDING, 60, 190),      // concave building over the bottom-left corner
        translate(rotate(rectangle(0, 0, 70, 40), pi / 8, {35, 20}), 270, 30),  // over a corner
        translate(scale(BUILDING, 1.3, 1.3, {0, 0}), 280, 200),               // mostly outside
    };

    fill(c, rectangle(view.xmin, view.ymin, view.xmax, view.ymax), PALE);
    int i = 0;
    for (const auto& shape : shapes) {
        const Polygon visible = clipPolygon(shape, view);
        if (!visible.empty()) fill(c, visible, CAR_COLORS[i % 4]);
        c.plot(polygonOutline(shape), visible.empty() ? RED : GREY);
        ++i;
    }
    rectOutline(c, int(view.xmin), int(view.ymin), int(view.xmax), int(view.ymax), BLUE);

    c.saveBmp(dir + "/06-sutherland-hodgman-clipping.bmp", SCALE);
}

// Filled shape from a closed ring of pixels: join the leftmost and rightmost
// ring pixels on every row (works for convex rings such as ellipses).
void fillRing(Canvas& c, const std::vector<Pixel>& ring, Color color) {
    if (ring.empty()) return;
    int top = ring[0].y, bottom = ring[0].y;
    for (const auto& p : ring) { top = std::min(top, p.y); bottom = std::max(bottom, p.y); }
    std::vector<int> left(bottom - top + 1, 1 << 30), right(bottom - top + 1, -(1 << 30));
    for (const auto& p : ring) {
        left[p.y - top] = std::min(left[p.y - top], p.x);
        right[p.y - top] = std::max(right[p.y - top], p.x);
    }
    for (int y = top; y <= bottom; ++y) line(c, left[y - top], y, right[y - top], y, color);
}

constexpr Color LEAF{60, 140, 70};
constexpr Color LEAF_DARK{35, 95, 45};
constexpr Color SHADOW{150, 170, 140};

// A tree seen from above: a shadow ellipse, then the canopy ellipse and its outline.
void tree(Canvas& c, int x, int y, int rx, int ry) {
    fillRing(c, midpointEllipse(x + 3, y + 4, rx, ry), SHADOW);
    const auto canopy = midpointEllipse(x, y, rx, ry);
    fillRing(c, canopy, LEAF);
    c.plot(canopy, LEAF_DARK);
}

// Image 7 — Midpoint ellipse: a roundabout island and tree canopies, plus a set
// of ellipses with different radii showing the two-region algorithm.
void renderEllipses(const std::string& dir) {
    Canvas c(W, H, GRASS);

    // Roads meeting at a roundabout.
    fill(c, rectangle(0, 120, W, 180), ASPHALT);
    fill(c, rectangle(170, 0, 230, H), ASPHALT);
    fillRing(c, midpointEllipse(200, 150, 70, 52), ASPHALT);

    // Central island: a filled ellipse with a white kerb and an inner flower bed.
    const auto island = midpointEllipse(200, 150, 38, 26);
    fillRing(c, island, GRASS);
    c.plot(island, WHITE);
    fillRing(c, midpointEllipse(200, 150, 14, 9), YELLOW);
    c.plot(midpointEllipse(200, 150, 54, 39), WHITE);     // lane divider ring

    // Trees of different sizes and proportions in the corners.
    tree(c, 40, 40, 18, 12);   tree(c, 95, 60, 12, 12);   tree(c, 60, 92, 9, 14);
    tree(c, 300, 45, 22, 14);  tree(c, 360, 80, 14, 10);
    tree(c, 330, 240, 16, 22);

    // Ellipse family: the same algorithm for wide, round and tall shapes.
    for (int k = 0; k < 5; ++k) {
        c.plot(midpointEllipse(80, 240, 10 + 9 * k, 40 - 7 * k), BLUE);
    }

    c.saveBmp(dir + "/07-midpoint-ellipse.bmp", SCALE);
}

// A turning route through the junction, as the four control points of a cubic Bézier.
struct Route {
    Point p0, p1, p2, p3;
    Color color;
};

// Left-hand traffic lanes: eastbound y = 135, westbound y = 165,
// southbound x = 215, northbound x = 185.
const Route ROUTES[] = {
    {{110, 135}, {170, 135}, {185, 120}, {185, 70},  {30, 110, 200}},   // west -> north (left turn)
    {{110, 135}, {205, 135}, {215, 150}, {215, 230}, {210, 60, 50}},    // west -> south (right turn)
    {{215, 70},  {215, 120}, {225, 135}, {290, 135}, {40, 150, 90}},    // north -> east (left turn)
    {{290, 165}, {200, 165}, {185, 150}, {185, 70},  {150, 80, 170}},   // east -> north (right turn)
};

// Image 8 — Curves: cubic Bézier turning routes through the junction, and a
// Catmull–Rom spline for a winding side road through its control points.
void renderCurves(const std::string& dir) {
    Canvas c(W, H, GRASS);
    fill(c, rectangle(0, 120, W, 180), ASPHALT);
    fill(c, rectangle(170, 0, 230, H), ASPHALT);

    // Bézier routes. For the first route, also show the control polygon:
    // the curve starts at P0 and ends at P3 but only bends towards P1 and P2.
    for (const auto& r : ROUTES) c.plot(polylinePixels(bezierPoints(r.p0, r.p1, r.p2, r.p3, 40)), r.color);
    const Route& shown = ROUTES[0];
    const std::vector<Point> hull{shown.p0, shown.p1, shown.p2, shown.p3};
    for (std::size_t i = 0; i + 1 < hull.size(); ++i) {
        dashedLine(c, int(hull[i].x), int(hull[i].y), int(hull[i + 1].x), int(hull[i + 1].y), 3, 3, WHITE);
    }
    for (const auto& p : hull) c.plot(midpointCircle(int(p.x), int(p.y), 3), YELLOW);

    // Catmull–Rom side road in the bottom-left corner: two kerb lines offset
    // from a centre line that passes through every control point (red circles).
    const std::vector<Point> control{{0, 285}, {40, 225}, {80, 262}, {125, 205}, {165, 240}};
    const auto centre = catmullRomSpline(control, 20);
    for (int offset = -6; offset <= 6; ++offset) {
        c.plot(polylinePixels(translate(centre, 0, offset)), ASPHALT);
    }
    c.plot(polylinePixels(centre), YELLOW);
    for (const auto& p : control) {
        disc(c, int(p.x), int(p.y), 3, RED);
    }

    // A second spline: a footpath through the top-right corner.
    const std::vector<Point> path{{240, 20}, {280, 60}, {330, 30}, {370, 80}, {395, 50}};
    c.plot(polylinePixels(catmullRomSpline(path, 20)), ROOF_A);
    for (const auto& p : path) c.plot(midpointCircle(int(p.x), int(p.y), 3), RED);

    c.saveBmp(dir + "/08-bezier-and-spline-curves.bmp", SCALE);
}

// Image 9 — The SmartFlow junction built from every technique so far:
//   scan-line fill (roads, buildings, cars), polygon transformations (placing cars),
//   Bresenham and dashed lines (markings), midpoint circles (signal lamps),
//   midpoint ellipses (tree canopies and shadows), Bézier routes
//   (car positions and headings), Catmull–Rom (side road), and both clipping
//   algorithms (everything is clipped to the camera viewport).
void renderJunctionScene(const std::string& dir) {
    Canvas c(W, H, WHITE);
    const double pi = std::acos(-1.0);
    const ClipRect view{10, 10, W - 11, H - 11};

    // Everything below is clipped to the viewport before it is drawn.
    auto area = [&](const Polygon& poly, Color color) {
        const Polygon visible = clipPolygon(poly, view);
        if (!visible.empty()) fill(c, visible, color);
    };
    auto edge = [&](const Polygon& poly, Color color) {
        for (std::size_t i = 0; i < poly.size(); ++i) {
            const Point& a = poly[i];
            const Point& b = poly[(i + 1) % poly.size()];
            if (auto s = clipLine(a.x, a.y, b.x, b.y, view)) {
                line(c, int(std::lround(s->x0)), int(std::lround(s->y0)),
                     int(std::lround(s->x1)), int(std::lround(s->y1)), color);
            }
        }
    };
    auto inView = [&](int x, int y) { return x >= view.xmin && x <= view.xmax && y >= view.ymin && y <= view.ymax; };

    // 1. Ground and roads (filled polygons), with a Catmull–Rom side road.
    area(rectangle(0, 0, W, H), GRASS);
    for (int offset = -5; offset <= 5; ++offset) {
        for (const auto& p : polylinePixels(translate(catmullRomSpline(
                 {{0, 270}, {50, 230}, {100, 255}, {140, 215}, {170, 205}}, 20), 0, offset))) {
            if (inView(p.x, p.y)) c.set(p.x, p.y, ASPHALT);
        }
    }
    area(rectangle(-20, 120, W + 20, 180), ASPHALT);
    area(rectangle(170, -20, 230, H + 20), ASPHALT);

    // 2. Markings: dashed centre lines, stop lines and zebra crossings.
    auto dashes = [&](int x0, int y0, int x1, int y1) {
        const auto pts = bresenhamLine(x0, y0, x1, y1);
        for (std::size_t i = 0; i < pts.size(); ++i) {
            if (i % 14 < 8 && inView(pts[i].x, pts[i].y)) c.set(pts[i].x, pts[i].y, YELLOW);
        }
    };
    dashes(0, 150, 150, 150);  dashes(250, 150, W, 150);
    dashes(200, 0, 200, 100);  dashes(200, 200, 200, H);
    for (int k = 0; k < 6; ++k) {
        area(rectangle(152, 123 + k * 10, 162, 128 + k * 10), WHITE);   // west crossing
        area(rectangle(238, 123 + k * 10, 248, 128 + k * 10), WHITE);   // east crossing
        area(rectangle(173 + k * 10, 102, 178 + k * 10, 112), WHITE);   // north crossing
        area(rectangle(173 + k * 10, 188, 178 + k * 10, 198), WHITE);   // south crossing
    }
    edge({{148, 120}, {148, 150}}, WHITE);   edge({{252, 150}, {252, 180}}, WHITE);
    edge({{200, 98}, {230, 98}}, WHITE);     edge({{170, 202}, {200, 202}}, WHITE);

    // 3. Buildings: concave and rotated footprints, partly outside the viewport.
    for (const auto& b : {translate(BUILDING, 30, 15), translate(scale(BUILDING, -1.2, 1, {0, 0}), 380, -5),
                          translate(rotate(rectangle(0, 0, 80, 45), pi / 14, {40, 22}), 285, 220)}) {
        area(b, ROOF_A);
        edge(b, ASPHALT);
    }

    // 4. Trees: ellipse canopies with shadows.
    for (const auto& t : {std::array<int, 4>{120, 60, 16, 12}, {110, 95, 10, 10}, {260, 85, 14, 11},
                          {360, 100, 12, 16}, {255, 260, 11, 9}, {20, 200, 13, 10}}) {
        if (inView(t[0], t[1])) tree(c, t[0], t[1], t[2], t[3]);
    }

    // 5. Cars. East-west traffic has green and is turning along the Bézier
    //    routes: the curve gives the position, and the direction to a slightly
    //    later point gives the heading. North-south traffic waits at red.
    int i = 0;
    auto car = [&](double x, double y, double heading) {
        const Polygon body = placeCar(x, y, heading);
        area(body, CAR_COLORS[i++ % 4]);
        edge(body, WHITE);
    };
    const struct { int route; double t; } moving[] = {{0, 0.7}, {1, 0.8}, {3, 0.6}};
    for (const auto& m : moving) {
        const Route& r = ROUTES[m.route];
        const Point at = cubicBezier(r.p0, r.p1, r.p2, r.p3, m.t);
        const Point ahead = cubicBezier(r.p0, r.p1, r.p2, r.p3, m.t + 0.01);
        car(at.x, at.y, std::atan2(ahead.y - at.y, ahead.x - at.x));
    }
    car(95, 135, 0);                 // eastbound, heading through on green
    car(330, 165, pi);               // westbound
    car(392, 165, pi);               // westbound, partly outside the viewport (clipped)
    car(215, 82, pi / 2);            // southbound, stopped at the red signal
    car(215, 54, pi / 2);
    car(185, 216, -pi / 2);          // northbound, stopped at the red signal
    car(185, 244, -pi / 2);
    car(185, 290, -pi / 2);          // partly outside the viewport (clipped)

    // 6. Signal heads at each corner: filled housings, midpoint-circle lamps.
    constexpr Color HOUSING{35, 35, 40};
    constexpr Color LAMP_ON[] = {{230, 50, 45}, {245, 170, 30}, {40, 200, 90}};
    const struct { int x, y, lit; } signals[] = {{150, 92, 2}, {236, 92, 0}, {150, 186, 0}, {236, 186, 2}};
    for (const auto& s : signals) {
        area(rectangle(s.x, s.y, s.x + 10, s.y + 26), HOUSING);
        for (int k = 0; k < 3; ++k) {
            if (k == s.lit) disc(c, s.x + 5, s.y + 5 + k * 8, 3, LAMP_ON[k]);
            else c.plot(midpointCircle(s.x + 5, s.y + 5 + k * 8, 3), GREY);
        }
    }

    // 7. Viewport frame.
    rectOutline(c, int(view.xmin), int(view.ymin), int(view.xmax), int(view.ymax), BLUE);

    c.saveBmp(dir + "/09-junction-scene.bmp", SCALE);
}

} // namespace

int main(int argc, char** argv) {
    const std::string dir = argc > 1 ? argv[1] : "output";
    std::filesystem::create_directories(dir);
    try {
        renderLines(dir);
        renderLineClipping(dir);
        renderCircles(dir);
        renderPolygons(dir);
        renderFill(dir);
        renderPolygonClipping(dir);
        renderEllipses(dir);
        renderCurves(dir);
        renderJunctionScene(dir);
    } catch (const std::exception& e) {
        std::cerr << "Render failed: " << e.what() << '\n';
        return 1;
    }
    std::cout << "Images written to " << dir << '\n';
    return 0;
}
