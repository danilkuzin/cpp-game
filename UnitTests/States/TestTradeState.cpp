#include "Bindings.h"
#include "Components.h"
#include "EntityStorage.h"
#include "GameItem.h"
#include "GameState.h"
#include "InputEvent.h"
#include "Inventory.h"
#include "PanelLayout.h"
#include "PanelRects.h"
#include "RectInt.h"
#include "RenderScene.h"
#include "Session.h"
#include "Support/Fixtures.h"
#include "Support/MakeSession.h"
#include "Texts.h"
#include "TradeState.h"
#include "Widget.h"
#include <gtest/gtest.h>
#include <string>

namespace {
entity::EntityId MakeHeroWithCha(game_states::Session &session, int cha) {
    auto hero_id = session.registry.entities.CreateEntity();
    session.registry.Storage<component::AttributesComponent>().Add(hero_id, WithCha(cha));
    session.registry.Storage<component::PlayerTag>().Add(hero_id, {});
    return hero_id;
}

entity::EntityId MakeNamedPricedItem(domain::Registry &registry, entity::EntityId owner,
                                     const std::string &name, int base_price) {
    auto id = MakeCarriedItem(registry, owner, 1);
    registry.Storage<component::NameComponent>().Add(id, {.name = name});
    registry.Storage<game_item::ValueComponent>().Add(id, {.base_price = base_price});
    return id;
}

} // namespace

TEST(TradeState, LeftClickOnAHeroItemRowSellsThatItem) {
    auto session = MakeTestSession();
    auto hero_id = MakeHeroWithCha(session, 100);
    auto shopkeeper_id = session.registry.entities.CreateEntity();
    session.registry.Storage<component::PurseComponent>().Add(shopkeeper_id, {.money = 1000});
    auto item_id = MakeNamedPricedItem(session.registry, hero_id, "Dagger", 100);
    game_states::TradeState state(session, shopkeeper_id);

    ui::RenderScene scene;
    state.BuildScene(scene);
    const auto [x, y] =
        RectCenter(ui::panel_layout::RowRect(ui::layout::kSidebarUpperPanelRect, 1));
    const auto widget = scene.HitTest(x, y);

    if (!widget.has_value()) {
        FAIL() << "expected a hit on the hero item row";
        return;
    }
    state.OnWidgetClick(widget.value(), ui::MouseButton::Left);

    EXPECT_EQ(session.registry.GetRef<game_item::OwnershipComponent>(item_id).owner, shopkeeper_id);
    EXPECT_GT(game_item::Money(session.registry, hero_id), 0);
}

TEST(TradeState, LeftClickOnAShopItemRowBuysThatItem) {
    auto session = MakeTestSession();
    auto hero_id = MakeHeroWithCha(session, 100);
    session.registry.Storage<component::PurseComponent>().Add(hero_id, {.money = 1000});
    for (int i = 0; i < 7; ++i) {
        MakeNamedPricedItem(session.registry, hero_id, "Junk", 1);
    }
    auto shopkeeper_id = session.registry.entities.CreateEntity();
    auto item_id = MakeNamedPricedItem(session.registry, shopkeeper_id, "Sword", 100);
    game_states::TradeState state(session, shopkeeper_id);

    ui::RenderScene scene;
    state.BuildScene(scene);
    const auto [x, y] =
        RectCenter(ui::panel_layout::RowRect(ui::layout::kSidebarLowerPanelRect, 1));
    const auto widget = scene.HitTest(x, y);

    if (!widget.has_value()) {
        FAIL() << "expected a hit on the shop item row";
        return;
    }
    EXPECT_EQ(widget.value().kind, ui::WidgetKind::LowerRow);
    state.OnWidgetClick(widget.value(), ui::MouseButton::Left);

    EXPECT_EQ(session.registry.GetRef<game_item::OwnershipComponent>(item_id).owner, hero_id);
    EXPECT_EQ(game_item::CarriedSize(session.registry, hero_id), 8);
}

TEST(TradeState, RightClickOnAShopRowDoesNothing) {
    auto session = MakeTestSession();
    auto hero_id = MakeHeroWithCha(session, 100);
    session.registry.Storage<component::PurseComponent>().Add(hero_id, {.money = 1000});
    auto shopkeeper_id = session.registry.entities.CreateEntity();
    auto item_id = MakeNamedPricedItem(session.registry, shopkeeper_id, "Sword", 100);
    game_states::TradeState state(session, shopkeeper_id);

    ui::RenderScene scene;
    state.BuildScene(scene);
    const auto [x, y] =
        RectCenter(ui::panel_layout::RowRect(ui::layout::kSidebarLowerPanelRect, 1));
    const auto widget = scene.HitTest(x, y);

    if (!widget.has_value()) {
        FAIL() << "expected a hit on the shop item row";
        return;
    }
    state.OnWidgetClick(widget.value(), ui::MouseButton::Right);

    EXPECT_EQ(session.registry.GetRef<game_item::OwnershipComponent>(item_id).owner, shopkeeper_id);
}

TEST(TradeState, FailedBuyLogsAndChangesNothing) {
    auto session = MakeTestSession();
    MakeHeroWithCha(session, 100);
    auto shopkeeper_id = session.registry.entities.CreateEntity();
    auto item_id = MakeNamedPricedItem(session.registry, shopkeeper_id, "Sword", 100);
    game_states::TradeState state(session, shopkeeper_id);

    ui::RenderScene scene;
    state.BuildScene(scene);
    const auto [x, y] =
        RectCenter(ui::panel_layout::RowRect(ui::layout::kSidebarLowerPanelRect, 1));
    const auto widget = scene.HitTest(x, y);

    if (!widget.has_value()) {
        FAIL() << "expected a hit on the shop item row";
        return;
    }
    state.OnWidgetClick(widget.value(), ui::MouseButton::Left);

    EXPECT_EQ(session.registry.GetRef<game_item::OwnershipComponent>(item_id).owner, shopkeeper_id);
    bool logged_cant_buy = false;
    for (const auto &message : session.runtime.log) {
        logged_cant_buy = logged_cant_buy || (message == texts::StrCantBuy());
    }
    EXPECT_TRUE(logged_cant_buy);
}

TEST(TradeState, CancelPops) {
    auto session = MakeTestSession();
    MakeHeroWithCha(session, 100);
    auto shopkeeper_id = session.registry.entities.CreateEntity();
    game_states::TradeState state(session, shopkeeper_id);

    auto result = state.OnAction(ui::Action::Cancel);

    if (!result.has_value()) {
        FAIL() << "expected Cancel to produce a GameState::Result";
        return;
    }
    EXPECT_EQ(result.value().type, game_states::GameState::Result::Type::Pop);
}
