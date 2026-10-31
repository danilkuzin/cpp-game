#include "GameOverState.h"
#include "Bindings.h"
#include "GameState.h"
#include "PanelRects.h"
#include "RenderScene.h"
#include "Session.h"
#include "Texts.h"
#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace game_states {
GameOverState::GameOverState(Session &session) : session_(session) {}

namespace {
std::string JoinEquipment(const std::vector<std::string> &equipment) {
    if (equipment.empty()) {
        return texts::StrGameOverNoEquipment();
    }
    std::string joined = equipment.front();
    for (std::size_t i = 1; i < equipment.size(); ++i) {
        joined += ", " + equipment[i];
    }
    return joined;
}
} // namespace

ui::InputContext GameOverState::InputContext() const { return ui::InputContext::Menu; }

std::optional<GameState::Result> GameOverState::OnAction(ui::Action action) {
    switch (action) {
    case ui::Action::Confirm:
    case ui::Action::Cancel:
        return GameState::Result{.type = GameState::Result::Type::Quit, .next_state = nullptr};
    default:
        break;
    }
    return std::nullopt;
}

void GameOverState::BuildScene(ui::RenderScene &scene) const {
    const auto &stats = session_.stats;
    scene.panels.push_back(
        {.rect = ui::layout::kGameOverPanelRect,
         .bg_color = ui::layout::kPanelBgColor,
         .text_color = ui::layout::kPanelTextColor,
         .rows = {texts::StrGameOver(), stats.hero_name,
                  texts::StrGameOverLevel() + std::to_string(stats.level),
                  texts::StrGameOverMonstersSlain() + std::to_string(stats.monsters_slain),
                  texts::StrHeroMoney() + std::to_string(stats.money),
                  texts::StrGameOverEquipment() + JoinEquipment(stats.equipment),
                  texts::StrGameOverPrompt()}});
}

} // namespace game_states
