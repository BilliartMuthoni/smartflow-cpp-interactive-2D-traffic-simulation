#include "canvas.hpp"

#include <algorithm>
#include <fstream>
#include <stdexcept>

namespace smartflow::graphics {

Canvas::Canvas(int width, int height, Color background)
    : width_(std::max(1, width)), height_(std::max(1, height)),
      pixels_(static_cast<std::size_t>(width_) * height_, background) {}

void Canvas::set(int x, int y, Color color) {
    if (x < 0 || y < 0 || x >= width_ || y >= height_) return;
    pixels_[static_cast<std::size_t>(y) * width_ + x] = color;
}

void Canvas::plot(const std::vector<Pixel>& pixels, Color color) {
    for (const auto& p : pixels) set(p.x, p.y, color);
}

void Canvas::saveBmp(const std::string& path, int scale) const {
    scale = std::max(1, scale);
    const int w = width_ * scale;
    const int h = height_ * scale;
    const int rowSize = (w * 3 + 3) / 4 * 4;  // each row is padded to a multiple of 4 bytes
    const std::uint32_t dataSize = static_cast<std::uint32_t>(rowSize) * h;

    std::ofstream out(path, std::ios::binary);
    if (!out) throw std::runtime_error("Cannot write " + path);

    // BMP numbers are little-endian.
    auto u16 = [&out](std::uint32_t v) { out.put(char(v & 0xFF)); out.put(char((v >> 8) & 0xFF)); };
    auto u32 = [&u16](std::uint32_t v) { u16(v & 0xFFFF); u16(v >> 16); };

    // 14-byte file header
    out.put('B'); out.put('M');
    u32(54 + dataSize); u32(0); u32(54);
    // 40-byte info header: size, width, height, planes, bits per pixel, no compression
    u32(40); u32(w); u32(h); u16(1); u16(24); u32(0);
    u32(dataSize); u32(2835); u32(2835); u32(0); u32(0);

    // Rows are stored bottom-up, and each pixel as blue, green, red.
    std::vector<char> row(rowSize, 0);
    for (int y = h - 1; y >= 0; --y) {
        for (int x = 0; x < w; ++x) {
            const Color& c = pixels_[static_cast<std::size_t>(y / scale) * width_ + x / scale];
            row[x * 3] = char(c.b);
            row[x * 3 + 1] = char(c.g);
            row[x * 3 + 2] = char(c.r);
        }
        out.write(row.data(), rowSize);
    }
}

} // namespace smartflow::graphics
