#include "graphics.hpp" 

#include <algorithm>  // std::max
#include <set>
#include <utility>    // std::pair
#include <utility> 

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

} // namespace smartflow::graphics
