#include "WorldSceneBuilder.h"
#include "Camera.h"
#include "Coordinates.h"
#include "Layout.h"
#include "Map.h"
#include "RectInt.h"
#include "RenderScene.h"
#include "Terrain.h"
#include <stdexcept>
#include <string>

namespace ui {
void WorldSceneBuilder::Build(RenderScene &scene, const map::Map &map, const Camera &camera,
                              const map::TerrainRegistry &terrain_registry, RectInt visible_area) {
    for (int x = camera.View().x; x < camera.View().x + camera.View().width; x++) {
        for (int y = camera.View().y; y < camera.View().y + camera.View().height; y++) {

            if (!map.Contains({.x = x, .y = y})) {
                throw std::invalid_argument(
                    "WorldSceneBuilder::Build: camera view (" + std::to_string(camera.View().x) +
                    ", " + std::to_string(camera.View().y) + ", " +
                    std::to_string(camera.View().width) + "x" +
                    std::to_string(camera.View().height) + ") reaches outside the map");
            }

            const auto &cell = map.At({.x = x, .y = y});

            if (!cell.explored) {
                continue;
            }

            const bool dim = !visible_area.ContainsPoint({.x = x, .y = y});
            scene.sprites.push_back(TileSprite(terrain_registry.Get(cell.terrain_id).sprite,
                                               {.x = x, .y = y}, camera.View(),
                                               ui::layout::kTerrainZIndex, dim));
        }
    }
}
} // namespace ui
