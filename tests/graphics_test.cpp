#include "graphics.hpp"

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <set>
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

int main() {
    testBresenham();
    testMidpointCircle();
    std::cout << "All graphics tests passed\n";
    return 0;
}
