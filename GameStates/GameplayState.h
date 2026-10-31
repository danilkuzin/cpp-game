#pragma once

#include "Bindings.h"
#include "GameState.h"
#include "InputEvent.h"
#include "RenderScene.h"
#include "Session.h"
#include "Vector2Int.h"
#include "Widget.h"
#include <cstddef>
#include <optional>

namespace game_states {
struct PendingTurn {
    bool intent_stamped = false;
    bool pickup_attempted = false;
    bool took_turn = false;
};

struct TargetingMode {
    std::size_t spell_slot = 0;
    Vector2Int origin;
    Vector2Int direction = Vector2Int::Up();
};

class GameplayState : public GameState {
public:
    explicit GameplayState(Session &session);

    void OnExit() override {};
    GameState::Result Update() override;
    void BuildScene(ui::RenderScene &scene) const override;
    ui::InputContext InputContext() const override;
    std::optional<GameState::Result> OnAction(ui::Action action) override;
    std::optional<GameState::Result> OnWidgetClick(ui::WidgetId widget,
                                                   ui::MouseButton button) override;
    void OnMouseClick(ui::MouseClickEvent event) override;
    void OnMouseMove(ui::MouseMoveEvent event, std::optional<ui::WidgetId> widget) override;

    [[nodiscard]] std::optional<Vector2Int> HoverTile() const { return hover_tile_; }

    [[nodiscard]] const std::optional<TargetingMode> &Targeting() const { return targeting_; }

private:
    void RefreshStats();
    void ConfirmTargeting();

    std::optional<GameState::Result> OnTargetingAction(ui::Action action);
    void CastSpell(std::size_t slot);

    game_states::Session &session_;
    PendingTurn pending_;
    std::optional<Vector2Int> hover_tile_;
    std::optional<TargetingMode> targeting_;
};
} // namespace game_states
