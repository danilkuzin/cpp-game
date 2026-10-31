#include "CharacterGenerationSceneBuilder.h"
#include "CharacterGenerationViewModel.h"
#include "Color.h"
#include "Layout.h"
#include "RenderScene.h"
#include <cstddef>

namespace ui {
void CharacterGenerationSceneBuilder::Build(RenderScene &scene,
                                            const CharacterGenerationViewModel &vm) {

    int y_pos = ui::layout::kTextFirstLine;
    scene.texts.push_back(
        {.text = vm.title, .x = ui::layout::kTextLeft, .y = y_pos, .text_color = ui::kBlack});
    y_pos += ui::layout::kTextTitleGap;
    scene.texts.push_back(
        {.text = vm.prompt, .x = ui::layout::kTextLeft, .y = y_pos, .text_color = ui::kBlack});
    y_pos += ui::layout::kTextLineStep;

    if (vm.mode == CharacterGenerationViewModel::Mode::SelectingOption) {
        for (size_t i = 0; i < vm.options.size(); ++i) {
            const ui::Color color = (i == vm.selected_idx) ? ui::kYellow : ui::kBlack;
            scene.texts.push_back({.text = vm.options[i],
                                   .x = ui::layout::kTextIndent,
                                   .y = y_pos,
                                   .text_color = color});
            y_pos += ui::layout::kTextLineStep;
        }
        for (const auto &s : vm.details) {
            scene.texts.push_back(
                {.text = s, .x = ui::layout::kTextIndent, .y = y_pos, .text_color = ui::kBlack});
            y_pos += ui::layout::kTextLineStep;
        }
    } else if (vm.mode == CharacterGenerationViewModel::Mode::TextInput) {
        scene.texts.push_back({.text = vm.input_text,
                               .x = ui::layout::kTextIndent,
                               .y = y_pos,
                               .text_color = ui::kYellow});
    }
}
} // namespace ui
