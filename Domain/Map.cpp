#include "Map.h"
#include "RectInt.h"
#include "Roll.h"
#include "Vector2Int.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

namespace map {
namespace {
constexpr int kStartingAreaSize = 8;

std::string OutOfRangeMessage(Vector2Int coord, int width, int height) {
    return "Map::At: (" + std::to_string(coord.x) + ", " + std::to_string(coord.y) +
           ") is outside a " + std::to_string(width) + "x" + std::to_string(height) + " map";
}
} // namespace

Map::Map(int width, int height, Cell fill)
    : cells_(static_cast<std::size_t>(std::max(0, width)) *
                 static_cast<std::size_t>(std::max(0, height)),
             fill),
      width_(std::max(0, width)), height_(std::max(0, height)) {}

RectInt Map::Bounds() const { return {.x = 0, .y = 0, .width = width_, .height = height_}; }

std::size_t Map::IndexOf(Vector2Int coord) const {
    if (!Contains(coord)) {
        throw std::out_of_range(OutOfRangeMessage(coord, width_, height_));
    }
    return (static_cast<std::size_t>(coord.x) * static_cast<std::size_t>(height_)) +
           static_cast<std::size_t>(coord.y);
}

const Cell &Map::At(Vector2Int coord) const { return cells_[IndexOf(coord)]; }

Cell &Map::At(Vector2Int coord) { return cells_[IndexOf(coord)]; }

RectInt Map::GetStartingArea() const {
    return {.x = (width_ - kStartingAreaSize) / 2,
            .y = (height_ - kStartingAreaSize) / 2,
            .width = kStartingAreaSize,
            .height = kStartingAreaSize};
}

void Map::SetAreaVisible(RectInt rect) {
    for (int x = std::max(0, rect.x); x < std::min(width_, rect.x + rect.width); x++) {
        for (int y = std::max(0, rect.y); y < std::min(height_, rect.y + rect.height); y++) {
            At({.x = x, .y = y}).explored = true;
        }
    }
}

int TrapDamage(int max_hp) {
    return (roll::Dice({.count = 1, .sides = static_cast<int>(std::round(max_hp * 1.1))})) + 1;
}
} // namespace map
