#pragma once

#include "Camera.h"
#include "RectInt.h"
#include "Registry.h"
#include "RenderScene.h"
#include "Vector2Int.h"

namespace ui {
class GameSceneBuilder {
public:
    static void Build(RenderScene &scene, const domain::Registry &registry, const Camera &camera);
    static void BuildHover(RenderScene &scene, const domain::Registry &registry,
                           const Camera &camera, Vector2Int tile);
    static void BuildTargeting(RenderScene &scene, const domain::Registry &registry,
                               const Camera &camera, Vector2Int direction);

private:
    static void BuildEntities(RenderScene &scene, const domain::Registry &registry,
                              const Camera &camera, RectInt visible_area, int map_level);
};
} // namespace ui
