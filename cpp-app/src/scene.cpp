#include "scene.hpp"

#include "rlgl.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <string>

namespace smartflow {
namespace {

constexpr Color grass{222, 233, 222, 255};
constexpr Color pavement{193, 209, 197, 255};
constexpr Color road{66, 86, 89, 255};
constexpr Color linePaint{226, 235, 219, 255};
constexpr Color ink{67, 104, 89, 255};
constexpr Color mint{100, 222, 171, 255};
constexpr std::array<Color, 7> carColors{{
    {40, 197, 214, 255}, {243, 173, 75, 255}, {157, 126, 232, 255},
    {233, 120, 131, 255}, {88, 197, 160, 255}, {104, 157, 229, 255},
    {230, 215, 155, 255}
}};

Camera2D sceneCamera(Rectangle viewport, float zoom) {
    const float fit = std::min(viewport.width / 1000.0f, viewport.height / 800.0f);
    return {{viewport.x + viewport.width / 2.0f, viewport.y + viewport.height / 2.0f},
            {500.0f, 400.0f}, 0.0f, fit * std::max(0.1f, zoom)};
}

void rounded(float x, float y, float w, float h, float radius, Color color) {
    DrawRectangleRounded({x, y, w, h}, std::min(1.0f, 2 * radius / std::min(w, h)), 8, color);
}

void centered(const char* text, float x, float y, float size, Color color, float spacing = 1.0f) {
    const Vector2 measured = MeasureTextEx(GetFontDefault(), text, size, spacing);
    DrawTextEx(GetFontDefault(), text, {x - measured.x / 2, y}, size, spacing, color);
}

void dashed(Vector2 a, Vector2 b, float stroke, Color color, float dash = 16, float gap = 14) {
    const float dx = b.x - a.x, dy = b.y - a.y;
    const float distance = std::hypot(dx, dy);
    if (distance <= 0.001f) return;
    for (float s = 0; s < distance; s += dash + gap) {
        const float end = std::min(distance, s + dash);
        DrawLineEx({a.x + dx * s / distance, a.y + dy * s / distance},
                   {a.x + dx * end / distance, a.y + dy * end / distance}, stroke, color);
    }
}

void arrow(float x, float y, float angle) {
    rlPushMatrix();
    rlTranslatef(x, y, 0);
    rlRotatef(angle, 0, 0, 1);
    const Color paint{194, 211, 198, 165};
    DrawLineEx({-15, 0}, {15, 0}, 2.4f, paint);
    DrawLineEx({5, -8}, {15, 0}, 2.4f, paint);
    DrawLineEx({5, 8}, {15, 0}, 2.4f, paint);
    rlPopMatrix();
}

void building(float x, float y, float w, float h, Color wall, Color roof) {
    rounded(x + 7, y + 9, w, h, 7, {93, 125, 107, 30});
    rounded(x, y, w, h, 6, wall);
    rounded(x + 8, y + 8, w - 16, h - 16, 3, roof);
    for (float xx = x + 20; xx < x + w - 10; xx += 16)
        DrawLineEx({xx, y + 9}, {xx, y + h - 9}, 0.8f, {91, 125, 112, 24});
    for (float yy = y + 22; yy < y + h - 10; yy += 16)
        DrawLineEx({x + 9, yy}, {x + w - 9, yy}, 0.8f, {91, 125, 112, 24});
    rounded(x + w - 41, y + 18, 22, 29, 2, wall);
    for (int i = 0; i < 4; ++i)
        DrawLineEx({x + w - 38, y + 24 + i * 5.0f},
                   {x + w - 22, y + 24 + i * 5.0f}, 1.5f, roof);
    // A narrow lip and entrance suggest height using only 2D primitives.
    DrawLineEx({x + 7, y + 5}, {x + w - 7, y + 5}, 2, {241, 249, 236, 115});
    rounded(x + w * 0.45f, y + h - 4, w * 0.18f, 5, 1, {97, 133, 119, 150});
}

void tree(float x, float y, float r) {
    DrawEllipse(static_cast<int>(x + 4), static_cast<int>(y + 6), r, r * 0.8f,
                {87, 129, 100, 35});
    DrawCircleV({x, y}, r, {134, 177, 140, 255});
    DrawCircleV({x - 3, y - 4}, r * 0.71f, {160, 195, 154, 255});
    DrawCircleV({x - 5, y - 6}, r * 0.37f, {181, 211, 168, 255});
}

void bench(float x, float y) {
    rounded(x + 2, y + 3, 29, 8, 2, {97, 124, 101, 30});
    rounded(x, y, 29, 8, 2, {169, 153, 111, 255});
    DrawLineEx({x + 2, y + 4}, {x + 27, y + 4}, 1, {211, 196, 154, 255});
    for (float xx : {x + 4, x + 25}) {
        DrawLineEx({xx, y}, {xx, y - 4}, 2, {110, 134, 111, 255});
        DrawLineEx({xx, y + 8}, {xx, y + 12}, 2, {110, 134, 111, 255});
    }
}

void drawMap() {
    DrawRectangle(0, 0, 1000, 800, grass);
    // A sparse dot grid adds texture without external images or font dependencies.
    for (int x = 12; x < 1000; x += 24)
        for (int y = 12; y < 800; y += 24)
            DrawCircle(x, y, 0.75f, {123, 159, 128, 31});

    DrawRectangle(390, 0, 220, 800, pavement);
    DrawRectangle(0, 290, 1000, 220, pavement);
    DrawRectangle(410, 0, 180, 800, road);
    DrawRectangle(0, 310, 1000, 180, road);
    const Color curb{126, 152, 132, 255};
    DrawLineEx({417, 0}, {417, 300}, 2, curb);
    DrawLineEx({583, 0}, {583, 300}, 2, curb);
    DrawLineEx({417, 500}, {417, 800}, 2, curb);
    DrawLineEx({583, 500}, {583, 800}, 2, curb);
    DrawLineEx({0, 317}, {400, 317}, 2, curb);
    DrawLineEx({600, 317}, {1000, 317}, 2, curb);
    DrawLineEx({0, 483}, {400, 483}, 2, curb);
    DrawLineEx({600, 483}, {1000, 483}, 2, curb);

    const Color centrePaint{213, 195, 135, 220};
    dashed({500, 0}, {500, 280}, 2.8f, centrePaint);
    dashed({500, 520}, {500, 800}, 2.8f, centrePaint);
    dashed({0, 400}, {383, 400}, 2.8f, centrePaint);
    dashed({617, 400}, {1000, 400}, 2.8f, centrePaint);

    // These match the model's exact inbound stop-line coordinates.
    DrawLineEx({510, 270}, {578, 270}, 5, linePaint);
    DrawLineEx({630, 410}, {630, 478}, 5, linePaint);
    DrawLineEx({422, 530}, {490, 530}, 5, linePaint);
    DrawLineEx({370, 322}, {370, 390}, 5, linePaint);
    for (int i = 0; i < 8; ++i) {
        const float p = 420 + i * 21.0f, q = 320 + i * 21.0f;
        rounded(p, 289, 12, 17, 1, {222, 234, 217, 185});
        rounded(p, 494, 12, 17, 1, {222, 234, 217, 185});
        rounded(389, q, 17, 12, 1, {222, 234, 217, 185});
        rounded(594, q, 17, 12, 1, {222, 234, 217, 185});
    }
    arrow(540, 180, 90); arrow(460, 180, -90);
    arrow(540, 625, 90); arrow(460, 625, -90);
    arrow(240, 360, 0); arrow(240, 440, 180);
    arrow(765, 360, 0); arrow(765, 440, 180);

    rounded(75, 65, 257, 151, 12, {208, 222, 210, 255});
    building(94, 83, 128, 112, {152, 183, 171, 255}, {192, 213, 197, 255});
    building(235, 100, 78, 95, {175, 188, 167, 255}, {212, 218, 197, 255});
    rounded(670, 65, 255, 151, 12, {207, 222, 211, 255});
    building(692, 83, 210, 113, {145, 178, 175, 255}, {191, 211, 203, 255});
    rounded(74, 585, 265, 143, 12, {207, 222, 210, 255});
    building(94, 603, 100, 108, {173, 191, 168, 255}, {204, 219, 195, 255});
    building(209, 603, 108, 108, {140, 181, 157, 255}, {186, 212, 185, 255});

    rounded(671, 579, 254, 157, 20, {193, 216, 184, 255});
    DrawLineEx({697, 734}, {697, 650}, 15, {231, 236, 212, 255});
    DrawLineEx({697, 650}, {723, 620}, 15, {231, 236, 212, 255});
    DrawCircleV({697, 650}, 7.5f, {231, 236, 212, 255});
    DrawLineEx({723, 620}, {925, 620}, 15, {231, 236, 212, 255});
    DrawCircleV({723, 620}, 7.5f, {231, 236, 212, 255});
    DrawLineEx({798, 735}, {798, 579}, 15, {231, 236, 212, 255});
    DrawCircleV({800, 623}, 24, {138, 175, 149, 70});
    DrawCircleV({798, 620}, 24, {165, 198, 170, 255});
    DrawCircleV({798, 620}, 18, {111, 173, 164, 255});
    DrawCircleLines(798, 620, 13, {157, 206, 185, 255});
    DrawCircleV({798, 620}, 8, {183, 216, 193, 255});

    constexpr float trees[][3] = {
        {52,95,16}, {53,175,14}, {95,250,15}, {155,250,15}, {315,250,17},
        {353,105,17}, {351,179,15}, {650,104,16}, {645,184,14}, {720,251,16},
        {872,252,16}, {950,88,15}, {948,181,17}, {60,572,15}, {61,703,17},
        {351,626,16}, {348,704,15}, {271,759,17}, {150,758,14}, {710,688,20},
        {755,674,16}, {878,674,23}, {900,610,16}, {849,707,15}
    };
    for (const auto& t : trees) tree(t[0], t[1], t[2]);
    bench(190, 246); bench(781, 248); bench(275, 556); bench(651, 677);

    // North is fixed in world space; zoom never changes the simulation geometry.
    DrawLineEx({955, 53}, {955, 29}, 1.8f, ink);
    DrawLineEx({950, 36}, {955, 29}, 1.8f, ink);
    DrawLineEx({960, 36}, {955, 29}, 1.8f, ink);
    centered("N", 955, 13, 12, ink);
}

void drawLabels() {
    const Color label{94, 123, 99, 255};
    centered("SCIENCE BLOCK", 206, 223, 11, label);
    centered("CAMPUS LIBRARY", 801, 223, 11, label);
    centered("STUDENT CENTRE", 204, 559, 11, label);
    centered("GREEN COMMON", 801, 759, 11, label);
    centered("UNIVERSITY ROAD", 205, 407, 10, {192, 210, 193, 170});
    centered("UNIVERSITY ROAD", 800, 407, 10, {192, 210, 193, 170});
    rlPushMatrix();
    rlTranslatef(504, 130, 0); rlRotatef(-90, 0, 0, 1);
    centered("CAMPUS AVENUE", 0, 0, 10, {192, 210, 193, 170});
    rlPopMatrix();
    rlPushMatrix();
    rlTranslatef(504, 680, 0); rlRotatef(-90, 0, 0, 1);
    centered("CAMPUS AVENUE", 0, 0, 10, {192, 210, 193, 170});
    rlPopMatrix();
}

void drawRoutes() {
    for (const auto& route : routes()) {
        const Color color = route.turn == "left" ? Color{188, 229, 121, 175}
                          : route.turn == "right" ? Color{119, 211, 231, 175}
                                                : Color{116, 228, 177, 175};
        // Continue the dash phase across every arc-length sample, so curves do
        // not become a solid line merely because they have many short segments.
        for (std::size_t i = 1; i < route.samples.size(); ++i) {
            const auto& a = route.samples[i - 1];
            const auto& b = route.samples[i];
            const double length = b.s - a.s;
            if (length <= 0.000001) continue;
            double s = a.s;
            while (s < b.s - 0.000001) {
                const double phase = std::fmod(s + 0.000001, 14.0);
                const bool visible = phase < 7.0;
                const double next = std::min(b.s, s + (visible ? 7.0 : 14.0) - phase);
                if (next <= s) break;
                if (visible) {
                    const double t0 = (s - a.s) / length, t1 = (next - a.s) / length;
                    DrawLineEx({static_cast<float>(a.x + (b.x - a.x) * t0),
                                static_cast<float>(a.y + (b.y - a.y) * t0)},
                               {static_cast<float>(a.x + (b.x - a.x) * t1),
                                static_cast<float>(a.y + (b.y - a.y) * t1)}, 1.7f, color);
                }
                s = next + 0.000002;
            }
        }
    }
}

void drawBoundary(bool occupied) {
    const Color edge = occupied ? Color{246, 200, 100, 245} : Color{148, 213, 180, 210};
    DrawRectangle(390, 290, 220, 220, occupied ? Color{238, 185, 75, 25} : Color{108, 211, 165, 14});
    dashed({390,290}, {610,290}, 1.8f, edge, 8, 5);
    dashed({610,290}, {610,510}, 1.8f, edge, 8, 5);
    dashed({610,510}, {390,510}, 1.8f, edge, 8, 5);
    dashed({390,510}, {390,290}, 1.8f, edge, 8, 5);
    rounded(429, 391, 142, 18, 4, {36, 60, 57, 165});
    centered(occupied ? "JUNCTION RESERVED" : "JUNCTION CLEAR", 500, 395, 10, edge, 0.5f);
}

void drawSignals(const TrafficSimulation& simulation) {
    constexpr char approaches[] = {'N', 'E', 'S', 'W'};
    constexpr float positions[][2] = {{607, 266}, {633, 511}, {329, 528}, {307, 265}};
    constexpr Color on[] = {{246,129,119,255}, {246,193,106,255}, {118,227,173,255}};
    const char* states[] = {"red", "amber", "green"};
    for (int i = 0; i < 4; ++i) {
        const float x = positions[i][0], y = positions[i][1];
        const auto state = simulation.signal(approaches[i]);
        rounded(x - 2, y, 63, 24, 7, {83, 116, 99, 45});
        rounded(x - 3, y - 3, 64, 23, 7, {177, 199, 181, 255});
        rounded(x, y, 58, 17, 5, {31, 58, 55, 255});
        for (int j = 0; j < 3; ++j) {
            const bool active = state == states[j];
            if (active) DrawCircleV({x + 11 + j * 18.0f, y + 8.5f}, 6.5f, Fade(on[j], 0.15f));
            DrawCircleV({x + 11 + j * 18.0f, y + 8.5f}, 4.4f,
                        active ? on[j] : Color{66, 87, 78, 255});
            if (active) DrawCircleV({x + 10 + j * 18.0f, y + 7.5f}, 1.2f, {255,255,230,145});
        }
        const char text[] = {approaches[i], '\0'};
        centered(text, x + 29, y + 25, 11, ink);
    }
}

void drawCar(const Vehicle& vehicle, bool selected) {
    const int safeIndex = (vehicle.colorIndex % static_cast<int>(carColors.size())
                           + static_cast<int>(carColors.size())) % static_cast<int>(carColors.size());
    const Color color = carColors[static_cast<std::size_t>(safeIndex)];
    rlPushMatrix();
    rlTranslatef(static_cast<float>(vehicle.x), static_cast<float>(vehicle.y), 0);
    rlRotatef(static_cast<float>(vehicle.angle), 0, 0, 1);
    if (selected) {
        rounded(-23, -16, 46, 32, 9, {58, 231, 177, 50});
        DrawRectangleRoundedLinesEx({-22, -15, 44, 30}, 0.55f, 8, 1.8f, mint);
    }
    rounded(-14, -6, 33, 18, 5, {18, 37, 35, 55});
    rounded(-11, -10, 7, 3, 1, {30, 52, 52, 255});
    rounded(7, -10, 6, 3, 1, {30, 52, 52, 255});
    rounded(-11, 7, 7, 3, 1, {30, 52, 52, 255});
    rounded(7, 7, 6, 3, 1, {30, 52, 52, 255});
    rounded(-16, -8.5f, 32, 17, 5, color);
    DrawLineEx({-11, -7}, {10, -7}, 1, {255, 255, 255, 105});
    rounded(4, -6, 5, 12, 1.4f, {37, 70, 78, 225});
    rounded(-11, -5.5f, 4, 11, 1.2f, {37, 70, 78, 215});
    rounded(-6, -6, 9, 12, 2, {255, 255, 255, 40});
    rounded(13.5f, -6.5f, 2, 3.5f, 0.7f, {250, 246, 217, 255});
    rounded(13.5f, 3, 2, 3.5f, 0.7f, {250, 246, 217, 255});
    const Color tail = vehicle.speed < 3 ? Color{255, 121, 102, 255} : Color{157, 63, 70, 255};
    rounded(-16, -6, 2, 3, 0.5f, tail);
    rounded(-16, 3, 2, 3, 0.5f, tail);
    rlPopMatrix();
}

} // namespace

void drawScene(const TrafficSimulation& simulation, Rectangle viewport, float zoom,
               bool showRoutes, bool showLabels, bool showBoundary, int selectedId) {
    if (viewport.width <= 0 || viewport.height <= 0) return;
    DrawRectangleRec(viewport, {210, 226, 213, 255});
    const Camera2D camera = sceneCamera(viewport, zoom);
    const Vector2 worldTop = GetWorldToScreen2D({0, 0}, camera);
    const Vector2 worldBottom = GetWorldToScreen2D({1000, 800}, camera);
    const float left = std::max(viewport.x, worldTop.x);
    const float top = std::max(viewport.y, worldTop.y);
    const float right = std::min(viewport.x + viewport.width, worldBottom.x);
    const float bottom = std::min(viewport.y + viewport.height, worldBottom.y);
    // Clip both to the panel and to the 1000 x 800 world. Cars approaching from
    // outside the world must not appear in the aspect-ratio letterbox margins.
    BeginScissorMode(static_cast<int>(std::ceil(left)), static_cast<int>(std::ceil(top)),
                     static_cast<int>(std::floor(right) - std::ceil(left)),
                     static_cast<int>(std::floor(bottom) - std::ceil(top)));
    BeginMode2D(camera);
    drawMap();
    if (showLabels) drawLabels();
    if (showRoutes) drawRoutes();
    if (showBoundary) drawBoundary(simulation.intersectionOccupied());
    drawSignals(simulation);
    for (const auto& vehicle : simulation.vehicles) drawCar(vehicle, vehicle.id == selectedId);
    EndMode2D();
    EndScissorMode();
}

int pickVehicle(const TrafficSimulation& simulation, Rectangle viewport, float zoom, Vector2 mouse) {
    if (viewport.width <= 0 || viewport.height <= 0 || !CheckCollisionPointRec(mouse, viewport)) return -1;
    const Vector2 world = GetScreenToWorld2D(mouse, sceneCamera(viewport, zoom));
    if (world.x < 0 || world.x > 1000 || world.y < 0 || world.y > 800) return -1;
    // Reverse draw order: overlapping click targets select the topmost car.
    for (auto it = simulation.vehicles.rbegin(); it != simulation.vehicles.rend(); ++it) {
        const double radians = it->angle * 3.14159265358979323846 / 180.0;
        const double dx = world.x - it->x, dy = world.y - it->y;
        const double localX = dx * std::cos(radians) + dy * std::sin(radians);
        const double localY = -dx * std::sin(radians) + dy * std::cos(radians);
        if (std::abs(localX) <= 22 && std::abs(localY) <= 14) return it->id;
    }
    return -1;
}

} // namespace smartflow
