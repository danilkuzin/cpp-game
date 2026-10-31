#pragma once

#include "Bindings.h"
#include "GameState.h"
#include "RenderScene.h"
#include "Session.h"
#include <optional>

namespace game_states {

class GameOverState : public GameState {
public:
    explicit GameOverState(Session &session);

    void BuildScene(ui::RenderScene &scene) const override;

    [[nodiscard]] bool IsOpaque() const override { return false; }

    ui::InputContext InputContext() const override;
    std::optional<GameState::Result> OnAction(ui::Action action) override;

private:
    game_states::Session &session_;
};
} // namespace game_states
