#pragma once

#include <cstdint>
#include <string>

namespace motif {

struct Color {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 255;

    static Color fromRGB(uint8_t r, uint8_t g, uint8_t b);
    static Color fromHex(const std::string& hex);

    // Generate Motif 3D shadow colors from a base color
    Color topShadow() const;
    Color bottomShadow() const;
    Color selectColor() const;

    bool operator==(const Color& other) const;
    bool operator!=(const Color& other) const;
};

} // namespace motif
