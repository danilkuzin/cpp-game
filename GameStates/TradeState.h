#pragma once

#include "Bindings.h"
#include "EntityStorage.h"
#include "GameState.h"
#include "InputEvent.h"
#include "RenderScene.h"
#include "Session.h"
#include "Widget.h"
#include <optional>

namespace game_states {
class TradeState : public GameState {
public:
    TradeState(Session &session, entity::EntityId shopkeeper_id);

    void BuildScene(ui::RenderScene &scene) const override;

    [[nodiscard]] bool IsOpaque() const override { return false; }

    ui::InputContext InputContext() const override;
    std::optional<GameState::Result> OnAction(ui::Action action) override;
    std::optional<GameState::Result> OnWidgetClick(ui::WidgetId widget,
                                                   ui::MouseButton button) override;

private:
    void HandleHeroItemClick(entity::EntityId item_id);
    void HandleShopItemClick(entity::EntityId item_id);

    game_states::Session &session_;
    entity::EntityId shopkeeper_id_;
};
} // namespace game_states
