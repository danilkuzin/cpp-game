#pragma once

#include "CharacterGenerationViewModel.h"
#include "RenderScene.h"

namespace ui {
class CharacterGenerationSceneBuilder {
public:
    static void Build(RenderScene &scene, const CharacterGenerationViewModel &vm);
};
} // namespace ui
