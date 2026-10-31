#include "CharacterGenState.h"
#include "Bindings.h"
#include "CharacterGenerationSceneBuilder.h"
#include "CharacterGenerationViewModel.h"
#include "EntityFactory.h"
#include "GameState.h"
#include "GameplayState.h"
#include "Hero.h"
#include "InputEvent.h"
#include "Location.h"
#include "RenderScene.h"
#include "Session.h"
#include "Tables.h"
#include "Texts.h"
#include "WorldGenerator.h"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <utility>

namespace game_states {

static_assert(texts::ClassNames().size() == static_cast<std::size_t>(hero::HClass::MaxClasses) + 1,
              "class_names must have one entry per hero::HClass value");
static_assert(texts::RaceNames().size() == static_cast<std::size_t>(hero::Race::MaxRaces) + 1,
              "race_names must have one entry per hero::Race value");

CharacterGenState::CharacterGenState(Session &session) : session_(session) {}

namespace {
std::size_t CycleIndex(int current, std::size_t count, int delta) {
    return (current + count + delta) % count;
}
} // namespace

std::optional<GameState::Result> CharacterGenState::OnAction(ui::Action action) {
    if (action == ui::Action::PrevOption) {
        if (stage_ == Stage::ChoosingClass) {
            hero_generation_parameters_.hclass = static_cast<hero::HClass>(
                CycleIndex(std::to_underlying(hero_generation_parameters_.hclass),
                           texts::ClassNames().size(), -1));
        } else if (stage_ == Stage::ChoosingRace) {
            hero_generation_parameters_.race = static_cast<hero::Race>(
                CycleIndex(std::to_underlying(hero_generation_parameters_.race),
                           texts::RaceNames().size(), -1));
        }
    } else if (action == ui::Action::NextOption) {
        if (stage_ == Stage::ChoosingClass) {
            hero_generation_parameters_.hclass = static_cast<hero::HClass>(
                CycleIndex(std::to_underlying(hero_generation_parameters_.hclass),
                           texts::ClassNames().size(), +1));
        } else if (stage_ == Stage::ChoosingRace) {
            hero_generation_parameters_.race = static_cast<hero::Race>(
                CycleIndex(std::to_underlying(hero_generation_parameters_.race),
                           texts::RaceNames().size(), +1));
        }
    } else if (action == ui::Action::DeleteChar) {
        if (stage_ == Stage::TypingName) {
            if (!hero_generation_parameters_.name.empty()) {
                hero_generation_parameters_.name.pop_back();
            }
        }
    } else if (action == ui::Action::Confirm) {
        if (stage_ == Stage::ChoosingClass) {
            stage_ = Stage::ChoosingRace;
        } else if (stage_ == Stage::ChoosingRace) {
            stage_ = Stage::TypingName;
        } else if (stage_ == Stage::TypingName) {
            stage_ = Stage::Done;
        }
    }
    return std::nullopt;
}

ui::InputContext CharacterGenState::InputContext() const {
    if (stage_ == Stage::TypingName) {
        return ui::InputContext::TextEntry;
    }
    return ui::InputContext::Menu;
}

void CharacterGenState::OnTextInput(ui::TextInputEvent event) {
    if (stage_ == Stage::TypingName) {
        hero_generation_parameters_.name += event.text;
    }
}

void CharacterGenState::BuildScene(ui::RenderScene &scene) const {
    ui::CharacterGenerationViewModel vm;

    vm.title = "Character Creation";
    switch (stage_) {
    case Stage::ChoosingClass:
        vm.prompt = "Choose your class:";
        {
            const auto class_names = texts::ClassNames();
            vm.options.assign(class_names.begin(), class_names.end());
        }
        vm.mode = ui::CharacterGenerationViewModel::Mode::SelectingOption;
        vm.selected_idx =
            static_cast<std::uint8_t>(std::to_underlying(hero_generation_parameters_.hclass));
        for (size_t si = 0; si < static_cast<std::size_t>(hero::Skill::MaxSkills) + 1; si++) {
            auto min =
                hero::kClassSkillTable[si][std::to_underlying(hero_generation_parameters_.hclass)]
                    .first;
            auto increment =
                hero::kClassSkillTable[si][std::to_underlying(hero_generation_parameters_.hclass)]
                    .second;
            vm.details.push_back(texts::SkillNames()[si] + ": " + std::to_string(min) + ":" +
                                 std::to_string(increment) + ",");
        }
        vm.details.push_back("Item: " +
                             entity::GetStartingItemName(hero_generation_parameters_.hclass));
        break;
    case Stage::ChoosingRace:
        vm.prompt = "Choose your race:";
        {
            const auto race_names = texts::RaceNames();
            vm.options.assign(race_names.begin(), race_names.end());
        }
        vm.mode = ui::CharacterGenerationViewModel::Mode::SelectingOption;
        vm.selected_idx =
            static_cast<std::uint8_t>(std::to_underlying(hero_generation_parameters_.race));
        for (int i = 0; i < hero::kMaxChars; i++) {
            auto min =
                hero::kRaceTable[std::to_underlying(hero_generation_parameters_.race)][i].first;
            auto increment =
                hero::kRaceTable[std::to_underlying(hero_generation_parameters_.race)][i].second;
            vm.details.push_back(texts::CharNames()[i] + ": " + std::to_string(min) + ":" +
                                 std::to_string(increment) + ",");
        }
        break;
    case Stage::TypingName:
        vm.prompt = "Enter your name:";
        vm.input_text = hero_generation_parameters_.name;
        vm.mode = ui::CharacterGenerationViewModel::Mode::TextInput;
        break;
    default:
        break;
    }

    ui::CharacterGenerationSceneBuilder::Build(scene, vm);
}

GameState::Result CharacterGenState::Update() {
    if (stage_ != Stage::Done) {
        return {};
    }
    auto hero_id = domain::WorldGenerator::GenerateHero(
        session_.registry, hero_generation_parameters_, kStartingMapLevel, session_.db);
    const auto &hero_loc = session_.registry.GetRef<Location>(hero_id);
    session_.runtime.camera.CenterAt(hero_loc.coord);
    return {.type = GameState::Result::Type::Replace,
            .next_state = std::make_unique<GameplayState>(session_)};
}

} // namespace game_states
