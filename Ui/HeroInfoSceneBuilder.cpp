#include "HeroInfoSceneBuilder.h"
#include "Color.h"
#include "Components.h"
#include "Hero.h"
#include "Layout.h"
#include "Registry.h"
#include "RegistryQueries.h"
#include "RenderScene.h"
#include "Texts.h"
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>

namespace ui {
namespace {
void PushRow(RenderScene &scene, int &cur_line, const std::string &text) {
    scene.texts.push_back(
        {.text = text, .x = ui::layout::kTextIndent, .y = cur_line, .text_color = ui::kBlack});
    cur_line += ui::layout::kTextLineStep;
}

void PushHeading(RenderScene &scene, int &cur_line, const std::string &text) {
    scene.texts.push_back(
        {.text = text, .x = ui::layout::kTextLeft, .y = cur_line, .text_color = ui::kBlack});
    cur_line += ui::layout::kTextLineStep;
}
} // namespace

void HeroInfoSceneBuilder::Build(RenderScene &scene, const domain::Registry &registry) {
    auto cur_line = ui::layout::kTextFirstLine;
    PushHeading(scene, cur_line, "Hero Info:");
    auto hero_id =
        systems::PlayerId(registry).value(); // NOLINT(bugprone-unchecked-optional-access)

    const auto *hero_name_component = registry.Storage<component::NameComponent>().Get(hero_id);
    if (hero_name_component == nullptr) {
        throw std::runtime_error("Hero name null");
    }
    PushRow(scene, cur_line, "name: " + hero_name_component->name);

    if (const auto *xp = registry.Storage<hero::HeroExperience>().Get(hero_id)) {
        const auto class_idx = static_cast<std::uint8_t>(std::to_underlying(xp->hclass));
        PushRow(scene, cur_line,
                std::string(texts::ClassNames().at(class_idx)) + ", level " +
                    std::to_string(xp->level) + " (xp " + std::to_string(xp->xp) + ")");
    }

    if (const auto *attrs = registry.Storage<component::AttributesComponent>().Get(hero_id)) {
        PushHeading(scene, cur_line, "Chars:");
        for (std::size_t i = 0; i < attrs->chars.size(); i++) {
            PushRow(scene, cur_line,
                    texts::CharNames().at(i) + ": " + std::to_string(attrs->chars.at(i)));
        }
    }

    if (const auto *skills = registry.Storage<hero::HeroSkills>().Get(hero_id)) {
        PushHeading(scene, cur_line, "Skills:");
        for (std::size_t i = 0; i < skills->skills.size(); i++) {
            PushRow(scene, cur_line,
                    texts::SkillNames().at(i) + ": " + std::to_string(skills->skills.at(i)));
        }
    }
}
} // namespace ui
