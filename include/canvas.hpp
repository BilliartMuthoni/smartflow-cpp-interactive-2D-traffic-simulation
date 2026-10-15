#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "graphics.hpp"

namespace smartflow::graphics {

struct Color {
    std::uint8_t r;
    std::uint8_t g;
    std::uint8_t b;
};

// A plain RGB pixel buffer. The algorithms decide which pixels to colour;
// the canvas only stores them and writes the result to a BMP file.
// Origin is the top-left corner and y grows downwards, like screen coordinates.
class Canvas {
public:
    Canvas(int width, int height, Color background);

    [[nodiscard]] int width() const { return width_; }
    [[nodiscard]] int height() const { return height_; }

    // Pixels outside the canvas are ignored, so shapes may run off the edge.
    void set(int x, int y, Color color);
    void plot(const std::vector<Pixel>& pixels, Color color);

    // Writes an uncompressed 24-bit BMP. Each canvas pixel becomes a
    // scale x scale block so the individual rasterised pixels stay visible.
    void saveBmp(const std::string& path, int scale = 1) const;

private:
    int width_;
    int height_;
    std::vector<Color> pixels_;
};

} // namespace smartflow::graphics
