#include "Terrain.h"

namespace map {
TerrainRegistry::TerrainRegistry() {
    terrains = {Terrain{.passable = true, .sprite = "tile_grass.bmp"},
                Terrain{.passable = true, .sprite = "tile_ground.bmp"},
                Terrain{.passable = true, .sprite = "tile_trap.bmp"},
                Terrain{.passable = true, .sprite = "tile_live.bmp"},
                Terrain{.passable = false, .sprite = "tile_tree.bmp"},
                Terrain{.passable = false, .sprite = "tile_stone.bmp"}};
}
} // namespace map
