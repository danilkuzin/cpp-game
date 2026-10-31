#pragma once

#include <cstdint>

namespace ui {
struct Color {
    std::uint8_t r = 0, g = 0, b = 0;

    bool operator==(const Color &) const = default;
};

constexpr Color kBlack{.r = 0, .g = 0, .b = 0};
constexpr Color kYellow{.r = 255, .g = 255, .b = 0};
} // namespace ui
