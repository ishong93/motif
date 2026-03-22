#include <motif/graphics/Color.h>
#include <algorithm>
#include <stdexcept>

namespace motif {

Color Color::fromRGB(uint8_t r, uint8_t g, uint8_t b) {
    return {r, g, b, 255};
}

Color Color::fromHex(const std::string& hex) {
    std::string h = hex;
    if (!h.empty() && h[0] == '#') h = h.substr(1);
    if (h.size() != 6) throw std::invalid_argument("Invalid hex color: " + hex);

    auto toInt = [](const std::string& s, size_t pos) -> uint8_t {
        return static_cast<uint8_t>(std::stoi(s.substr(pos, 2), nullptr, 16));
    };

    return {toInt(h, 0), toInt(h, 2), toInt(h, 4), 255};
}

Color Color::topShadow() const {
    // Motif formula: lighten by ~40%
    auto lighten = [](uint8_t c) -> uint8_t {
        return static_cast<uint8_t>(std::min(255, c + (255 - c) * 40 / 100));
    };
    return {lighten(r), lighten(g), lighten(b), a};
}

Color Color::bottomShadow() const {
    // Motif formula: darken by ~40%
    auto darken = [](uint8_t c) -> uint8_t {
        return static_cast<uint8_t>(c * 60 / 100);
    };
    return {darken(r), darken(g), darken(b), a};
}

Color Color::selectColor() const {
    // Motif select color: midpoint between base and bottom shadow
    return {
        static_cast<uint8_t>((r + r * 60 / 100) / 2),
        static_cast<uint8_t>((g + g * 60 / 100) / 2),
        static_cast<uint8_t>((b + b * 60 / 100) / 2),
        a
    };
}

bool Color::operator==(const Color& other) const {
    return r == other.r && g == other.g && b == other.b && a == other.a;
}

bool Color::operator!=(const Color& other) const {
    return !(*this == other);
}

} // namespace motif
