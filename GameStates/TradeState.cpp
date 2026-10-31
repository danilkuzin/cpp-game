#include "TradeState.h"
#include "Bindings.h"
#include "EntityStorage.h"
#include "EventFormat.h"
#include "GameItem.h"
#include "GameState.h"
#include "InputEvent.h"
#include "Inventory.h"
#include "PanelRects.h"
#include "Registry.h"
#include "RenderScene.h"
#include "Session.h"
#include "SidebarPanel.h"
#include "Texts.h"
#include "Trade.h"
#include "Widget.h"
#include <optional>
#include <string>
#include <vector>

namespace game_states {
TradeState::TradeState(Session &session, entity::EntityId shopkeeper_id)
    : session_(session), shopkeeper_id_(shopkeeper_id) {}

namespace {

std::string PricedLine(const domain::Registry &registry, entity::EntityId item_id) {
    std::string line = "- " + registry.GetRef<component::NameComponent>(item_id).name;
    if (const auto *value = registry.Storage<game_item::ValueComponent>().Get(item_id)) {
        line += " (" + std::to_string(value->base_price) + ")";
    }
    return line;
}
} // namespace

ui::InputContext TradeState::InputContext() const { return ui::InputContext::Menu; }

std::optional<GameState::Result> TradeState::OnAction(ui::Action action) {
    switch (action) {
    case ui::Action::Cancel:
        return GameState::Result{.type = GameState::Result::Type::Pop, .next_state = nullptr};
        break;

    default:
        break;
    }
    return std::nullopt;
};

std::optional<GameState::Result> TradeState::OnWidgetClick(ui::WidgetId widget,
                                                           ui::MouseButton button) {
    if (!widget.entity.has_value()) {
        return std::nullopt;
    }
    const auto item_id = widget.entity.value();
    switch (widget.kind) {
    case ui::WidgetKind::UpperRow:
        if (button == ui::MouseButton::Left) {
            HandleHeroItemClick(item_id);
        }
        break;
    case ui::WidgetKind::LowerRow:
        if (button == ui::MouseButton::Left) {
            HandleShopItemClick(item_id);
        }
        break;
    default:
        break;
    }
    return std::nullopt;
}

void TradeState::HandleHeroItemClick(entity::EntityId item_id) {
    auto hero_id = HeroId(session_);

    if (!trade::Sell(session_.registry,
                     {.hero_id = hero_id, .shopkeeper_id = shopkeeper_id_, .item_id = item_id})) {
        Log(session_, texts::StrCantSell());
    }
}

void TradeState::HandleShopItemClick(entity::EntityId item_id) {
    auto hero_id = HeroId(session_);

    if (!trade::Buy(session_.registry,
                    {.hero_id = hero_id, .shopkeeper_id = shopkeeper_id_, .item_id = item_id})) {
        Log(session_, texts::StrCantBuy());
    }
}

void TradeState::BuildScene(ui::RenderScene &scene) const {

    ui::PushSidebarBackdrop(scene);

    const auto hero_id = HeroId(session_);
    const auto shop_item_ids = game_item::OwnedItems(session_.registry, shopkeeper_id_);
    // Equipped gear isn't offered for sale: you can't sell the sword in your hand without taking it
    // off first. The shop's own list is unfiltered — a shopkeeper wears nothing.
    const auto hero_inventory_ids = game_item::UnequippedItems(session_.registry, hero_id);

    std::vector<ui::ListRow> hero_rows;
    hero_rows.reserve(hero_inventory_ids.size());
    for (auto item_id : hero_inventory_ids) {
        hero_rows.push_back({.text = PricedLine(session_.registry, item_id), .entity = item_id});
    }
    ui::PushListPanel(scene, ui::layout::kSidebarUpperPanelRect, texts::StrHeroItems(), hero_rows,
                      ui::WidgetKind::UpperRow);

    std::vector<ui::ListRow> shop_rows;
    shop_rows.reserve(shop_item_ids.size());
    for (auto item_id : shop_item_ids) {
        shop_rows.push_back({.text = PricedLine(session_.registry, item_id), .entity = item_id});
    }
    ui::PushListPanel(scene, ui::layout::kSidebarLowerPanelRect, texts::StrShop(), shop_rows,
                      ui::WidgetKind::LowerRow);

    ui::PushLogPanel(scene, ui::layout::kSidebarLogPanelRect, session_.runtime.log);
}

} // namespace game_states
