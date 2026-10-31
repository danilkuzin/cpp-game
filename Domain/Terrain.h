#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace map {
struct Terrain {
    bool passable = false;
    std::string sprite;
};

using TerrainId = std::uint16_t;

struct TerrainRegistry {
    std::vector<Terrain> terrains;
    TerrainRegistry();

    [[nodiscard]] const Terrain &Get(TerrainId id) const { return terrains[id]; }

    [[nodiscard]] bool IsPassable(TerrainId id) const { return Get(id).passable; }
};
} // namespace map
