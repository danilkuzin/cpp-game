#pragma once

#include "Camera.h"
#include "Map.h"
#include "RectInt.h"
#include "RenderScene.h"
#include "Terrain.h"

namespace ui {
class WorldSceneBuilder {
public:
    static void Build(RenderScene &scene, const map::Map &map, const Camera &camera,
                      const map::TerrainRegistry &terrain_registry, RectInt visible_area);
};
} // namespace ui
