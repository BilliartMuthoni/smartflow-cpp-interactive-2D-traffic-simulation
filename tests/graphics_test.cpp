#include "graphics.hpp"

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <set>
#include <stdexcept>
#include <utility>

namespace g = smartflow::graphics;   

void testBresenham() {
    auto pts = g::bresenhamLine(0, 0, 5, 2);
    int expected[6][2] = {{0,0},{1,0},{2,1},{3,1},{4,2},{5,2}};
    assert(pts.size() == 6);
    for (int i = 0; i < 6; i++) {
        assert(pts[i].x == expected[i][0] && pts[i].y == expected[i][1]);
    }
    assert(g::bresenhamLine(3, 3, 3, 3).size() == 1);   // single point
    assert(g::bresenhamLine(0, 0, 2, 7).size() == 8);   // steep line

    auto line = g::bresenhamLine(-4, 9, 6, -3);          // no gaps
    for (size_t i = 1; i < line.size(); i++) {
        assert(std::abs(line[i].x - line[i-1].x) <= 1);
        assert(std::abs(line[i].y - line[i-1].y) <= 1);
    }
    std::cout << "Bresenham: all tests passed\n";
}

void testMidpointCircle() {
    // 1. Matches the hand trace: r = 5 gives 28 unique pixels
    auto c = g::midpointCircle(0, 0, 5);
    assert(c.size() == 28);

    // 2. Accuracy: every pixel is within one pixel of the true circle
    for (const auto& p : c) {
        double dist = std::sqrt(double(p.x * p.x + p.y * p.y));
        assert(std::abs(dist - 5.0) < 1.0);
    }

    // 3. No duplicates: the set size equals the vector size
    std::set<std::pair<int, int>> unique;
    for (const auto& p : c) unique.insert({p.x, p.y});
    assert(unique.size() == c.size());

    // 4. Symmetry: if (x, y) is on the circle, (y, x) must be too
    for (const auto& p : c) assert(unique.count({p.y, p.x}) == 1);

    // 5. Centre offset: the first pixel is (cx + r, cy)
    auto shifted = g::midpointCircle(10, -3, 5);
    assert(shifted.size() == 28);
    assert(shifted[0].x == 15 && shifted[0].y == -3);

    // 6. Edge cases
    assert(g::midpointCircle(0, 0, 0).size() == 1);    // just the centre
    assert(g::midpointCircle(0, 0, -4).size() == 1);   // negative treated as 0

    std::cout << "Midpoint circle: all tests passed\n";
}

bool near(double a, double b) { return std::abs(a - b) < 1e-9; }

bool sameSegment(const std::optional<g::Segment>& s, double x0, double y0, double x1, double y1) {
    return s && near(s->x0, x0) && near(s->y0, y0) && near(s->x1, x1) && near(s->y1, y1);
}

void testCohenSutherland() {
    const g::ClipRect box{0, 0, 10, 10};

    // 1. Fully inside: returned unchanged (trivial accept)
    assert(sameSegment(g::clipLine(2, 3, 8, 9, box), 2, 3, 8, 9));

    // 2. Crossing the window: cut at both edges
    assert(sameSegment(g::clipLine(-5, 5, 15, 5, box), 0, 5, 10, 5));    // horizontal
    assert(sameSegment(g::clipLine(5, 15, 5, -5, box), 5, 10, 5, 0));    // vertical
    assert(sameSegment(g::clipLine(-5, -5, 15, 15, box), 0, 0, 10, 10)); // diagonal

    // 3. Fully outside on one side: rejected (both codes share a bit)
    assert(!g::clipLine(11, 1, 11, 9, box));
    assert(!g::clipLine(-5, -1, 15, -1, box));

    // 4. Outside in different regions but missing the corner: rejected after clipping
    assert(!g::clipLine(-5, 6, 6, 15, box));

    // 5. Touching the boundary counts as visible (inclusive edges)
    assert(sameSegment(g::clipLine(-5, 0, 15, 0, box), 0, 0, 10, 0));

    // 6. Direction does not matter: reversing the input reverses the output
    auto forward = g::clipLine(-3, 2, 14, 8, box);
    auto backward = g::clipLine(14, 8, -3, 2, box);
    assert(forward && sameSegment(backward, forward->x1, forward->y1, forward->x0, forward->y0));

    // 7. Invalid input is rejected
    bool threw = false;
    try { (void)g::clipLine(0, 0, 1, 1, {10, 0, 0, 10}); } catch (const std::invalid_argument&) { threw = true; }
    assert(threw);

    std::cout << "Cohen-Sutherland clipping: all tests passed\n";
}

bool samePoint(const g::Point& p, double x, double y) { return near(p.x, x) && near(p.y, y); }

void testPolygonTransforms() {
    const g::Polygon rect{{0, 0}, {4, 0}, {4, 3}, {0, 3}};

    // 1. Shoelace area of a 4 x 3 rectangle is 12; reversing the order flips the sign
    assert(near(g::signedArea(rect), 12));
    const g::Polygon reversed{{0, 3}, {4, 3}, {4, 0}, {0, 0}};
    assert(near(g::signedArea(reversed), -12));

    // 2. Translation moves every vertex and keeps the area
    auto moved = g::translate(rect, 10, -2);
    assert(samePoint(moved[0], 10, -2) && samePoint(moved[2], 14, 1));
    assert(near(g::signedArea(moved), 12));

    // 3. Rotating 90 degrees about the origin maps (4, 0) to (0, 4); area is kept
    const double quarter = std::acos(-1.0) / 2;
    auto turned = g::rotate(rect, quarter, {0, 0});
    assert(samePoint(turned[1], 0, 4));
    assert(near(g::signedArea(turned), 12));

    // 4. Rotating about the centre keeps the centre fixed
    auto centre = g::vertexCentre(rect);
    assert(samePoint(centre, 2, 1.5));
    auto spun = g::rotate(rect, 0.7, centre);
    assert(samePoint(g::vertexCentre(spun), 2, 1.5));

    // 5. Scaling by (2, 3) multiplies the area by 6
    auto bigger = g::scale(rect, 2, 3, {0, 0});
    assert(samePoint(bigger[2], 8, 9));
    assert(near(g::signedArea(bigger), 72));

    // 6. The outline is closed: every vertex is on it
    auto outline = g::polygonOutline(rect);
    for (const auto& v : rect) {
        bool found = false;
        for (const auto& p : outline) found = found || (p.x == int(v.x) && p.y == int(v.y));
        assert(found);
    }

    std::cout << "Polygon transformations: all tests passed\n";
}

bool contains(const std::vector<g::Pixel>& pixels, int x, int y) {
    for (const auto& p : pixels) if (p.x == x && p.y == y) return true;
    return false;
}

void testScanlineFill() {
    // 1. A 4 x 3 rectangle covers exactly 12 pixel centres
    const g::Polygon rect{{0, 0}, {4, 0}, {4, 3}, {0, 3}};
    auto filled = g::scanlineFill(rect);
    assert(filled.size() == 12);
    assert(contains(filled, 0, 0) && contains(filled, 3, 2));
    assert(!contains(filled, 4, 0) && !contains(filled, 0, 3));

    // 2. Vertex order does not matter
    const g::Polygon reversed{{0, 3}, {4, 3}, {4, 0}, {0, 0}};
    assert(g::scanlineFill(reversed).size() == 12);

    // 3. Concave L-shape: the notch stays empty, the arms are filled
    const g::Polygon ell{{0, 0}, {6, 0}, {6, 2}, {2, 2}, {2, 6}, {0, 6}};
    auto l = g::scanlineFill(ell);
    assert(l.size() == 6 * 2 + 2 * 4);         // bottom bar 12 + upright 8
    assert(contains(l, 5, 1) && contains(l, 1, 5));
    assert(!contains(l, 4, 4));                // inside the notch

    // 4. No pixel is filled twice
    std::set<std::pair<int, int>> unique;
    for (const auto& p : l) unique.insert({p.x, p.y});
    assert(unique.size() == l.size());

    // 5. Every filled pixel centre of a triangle lies inside it
    const g::Polygon tri{{0, 0}, {10, 0}, {0, 10}};
    for (const auto& p : g::scanlineFill(tri)) {
        assert(p.x >= 0 && p.y >= 0 && (p.x + 0.5) + (p.y + 0.5) <= 10);
    }

    // 6. Degenerate input fills nothing
    assert(g::scanlineFill({{0, 0}, {5, 5}}).empty());

    std::cout << "Scan-line fill: all tests passed\n";
}

int main() {
    testBresenham();
    testMidpointCircle();
    testCohenSutherland();
    testPolygonTransforms();
    testScanlineFill();
    std::cout << "All graphics tests passed\n";
    return 0;
}
