#pragma once

#include "Registry.h"
#include "RenderScene.h"

namespace ui {
class HeroInfoSceneBuilder {
public:
    static void Build(RenderScene &scene, const domain::Registry &registry);
};
} // namespace ui
