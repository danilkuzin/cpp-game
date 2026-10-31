#pragma once

#include "Bindings.h"
#include "GameState.h"
#include "RenderScene.h"
#include "Session.h"
#include <optional>

namespace game_states {
class HeroInfoState : public GameState {
public:
    explicit HeroInfoState(Session &session);
    void OnEnter() override {};
    void OnExit() override {};
    void BuildScene(ui::RenderScene &scene) const override;
    ui::InputContext InputContext() const override;
    std::optional<GameState::Result> OnAction(ui::Action action) override;

private:
    game_states::Session &session_;
};
} // namespace game_states
