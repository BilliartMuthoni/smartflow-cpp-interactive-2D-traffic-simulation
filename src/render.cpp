// Renders the checkpoint images using only our own algorithms and the BMP canvas.
// Usage: render [output-folder]   (default: output)

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

} // namespace

int main(int argc, char** argv) {
    const std::string dir = argc > 1 ? argv[1] : "output";
    std::filesystem::create_directories(dir);
    try {
        renderLines(dir);
        renderLineClipping(dir);
    } catch (const std::exception& e) {
        std::cerr << "Render failed: " << e.what() << '\n';
        return 1;
    }
    std::cout << "Images written to " << dir << '\n';
    return 0;
}
