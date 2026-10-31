#pragma once

#include "RectInt.h"
#include "Terrain.h"
#include "Vector2Int.h"
#include <cstddef>
#include <vector>

namespace map {
struct Cell {
    TerrainId terrain_id = 0;
    bool explored = false;
};

class Map {
public:
    Map() = default;
    Map(int width, int height, Cell fill);

    [[nodiscard]] RectInt Bounds() const;

    [[nodiscard]] bool Contains(Vector2Int coord) const { return Bounds().ContainsPoint(coord); }

    [[nodiscard]] const Cell &At(Vector2Int coord) const;
    [[nodiscard]] Cell &At(Vector2Int coord);

    [[nodiscard]] RectInt GetStartingArea() const;

    void SetAreaVisible(RectInt rect);

    [[nodiscard]] int Width() const { return width_; }

    [[nodiscard]] int Height() const { return height_; }

private:
    [[nodiscard]] std::size_t IndexOf(Vector2Int coord) const;

    std::vector<Cell> cells_;
    int width_ = 0;
    int height_ = 0;
};

struct TrapComponent {
    bool active = false;
    int difficulty = 0;
};

struct HealingSourceComponent {
    int amount = 0;
};

int TrapDamage(int max_hp);

} // namespace map
