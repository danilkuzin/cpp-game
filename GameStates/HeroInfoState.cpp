#include "HeroInfoState.h"
#include "Bindings.h"
#include "GameState.h"
#include "HeroInfoSceneBuilder.h"
#include "RenderScene.h"
#include "Session.h"
#include <optional>

namespace game_states {

HeroInfoState::HeroInfoState(Session &session) : session_(session) {}

ui::InputContext HeroInfoState::InputContext() const { return ui::InputContext::Menu; }

std::optional<GameState::Result> HeroInfoState::OnAction(ui::Action action) {
    switch (action) {
    case ui::Action::OpenHeroInfo:
    case ui::Action::Cancel:
        return GameState::Result{.type = GameState::Result::Type::Pop, .next_state = nullptr};
    default:
        break;
    }
    return std::nullopt;
}

void HeroInfoState::BuildScene(ui::RenderScene &scene) const {
    ui::HeroInfoSceneBuilder::Build(scene, session_.registry);
}

} // namespace game_states
