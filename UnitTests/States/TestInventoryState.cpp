#include "Components.h"
#include "EntityStorage.h"
#include "GameItem.h"
#include "GameState.h"
#include "InputEvent.h"
#include "Inventory.h"
#include "InventoryState.h"
#include "Location.h"
#include "PanelLayout.h"
#include "PanelRects.h"
#include "RectInt.h"
#include "RenderScene.h"
#include "Session.h"
#include "Spell.h"
#include "Support/Fixtures.h"
#include "Support/MakeSession.h"
#include <algorithm>
#include <gtest/gtest.h>
#include <string>
#include <utility>

namespace {
entity::EntityId MakeHero(game_states::Session &session) {
    auto hero_id =
        MakeEntityAt(session.registry, Location{.coord = {.x = 0, .y = 0}, .map_level = 0});
    session.registry.Storage<component::PlayerTag>().Add(hero_id, {});
    return hero_id;
}

entity::EntityId MakeNamedEquippable(game_states::Session &session, entity::EntityId owner,
                                     const std::string &name) {
    auto id = MakeCarriedEquippableItem(session.registry, owner, game_item::EquipSlot::Chest);
    session.registry.Storage<component::NameComponent>().Add(id, {.name = name});
    return id;
}

entity::EntityId MakeNamedMagicItem(game_states::Session &session, entity::EntityId owner,
                                    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
                                    const std::string &name, spell::SpellId spell_id, int charges) {
    auto id = MakeCarriedMagicItem(session.registry, owner, std::move(spell_id), charges);
    session.registry.Storage<component::NameComponent>().Add(id, {.name = name});
    return id;
}

} // namespace

TEST(InventoryState, LeftClickOnAnItemRowEquipsThatItem) {
    auto session = MakeTestSession();
    auto hero_id = MakeHero(session);
    auto first_item = MakeNamedEquippable(session, hero_id, "First");
    auto second_item = MakeNamedEquippable(session, hero_id, "Second");
    game_states::InventoryState state(session);

    ui::RenderScene scene;
    state.BuildScene(scene);
    const auto [x, y] =
        RectCenter(ui::panel_layout::RowRect(ui::layout::kSidebarUpperPanelRect, 2));
    const auto widget = scene.HitTest(x, y);

    if (!widget.has_value() || !widget.value().entity.has_value()) {
        FAIL() << "expected a hit on an item row with an entity";
        return;
    }
    const auto clicked_item = widget.value().entity.value();
    const auto other_item = (clicked_item == first_item) ? second_item : first_item;
    state.OnWidgetClick(widget.value(), ui::MouseButton::Left);

    EXPECT_TRUE(session.registry.Storage<game_item::EquippedComponent>().Has(clicked_item));
    EXPECT_FALSE(session.registry.Storage<game_item::EquippedComponent>().Has(other_item));
}

TEST(InventoryState, RightClickOnAnItemRowDropsIt) {
    auto session = MakeTestSession();
    auto hero_id = MakeHero(session);
    auto item_id = MakeNamedEquippable(session, hero_id, "Sword");
    game_states::InventoryState state(session);

    ui::RenderScene scene;
    state.BuildScene(scene);
    const auto [x, y] =
        RectCenter(ui::panel_layout::RowRect(ui::layout::kSidebarUpperPanelRect, 1));
    const auto widget = scene.HitTest(x, y);

    if (!widget.has_value()) {
        FAIL() << "expected a hit on the item row";
        return;
    }
    state.OnWidgetClick(widget.value(), ui::MouseButton::Right);

    EXPECT_FALSE(session.registry.Storage<game_item::OwnershipComponent>().Has(item_id));
    EXPECT_TRUE(session.registry.Storage<Location>().Has(item_id));
}

TEST(InventoryState, LeftClickOnAnEquippedSlotRowUnequipsIt) {
    auto session = MakeTestSession();
    auto hero_id = MakeHero(session);
    auto item_id =
        MakeCarriedEquippableItem(session.registry, hero_id, game_item::EquipSlot::Chest);
    session.registry.Storage<component::NameComponent>().Add(item_id, {.name = "Armor"});
    ASSERT_TRUE(game_item::Equip(session.registry, {.item = item_id, .actor = hero_id},
                                 game_item::EquipSlot::Chest));
    game_states::InventoryState state(session);

    ui::RenderScene scene;
    state.BuildScene(scene);
    const auto [x, y] =
        RectCenter(ui::panel_layout::RowRect(ui::layout::kSidebarLowerPanelRect, 1));
    const auto widget = scene.HitTest(x, y);

    if (!widget.has_value()) {
        FAIL() << "expected a hit on the equipped slot row";
        return;
    }
    state.OnWidgetClick(widget.value(), ui::MouseButton::Left);

    EXPECT_FALSE(session.registry.Storage<game_item::EquippedComponent>().Has(item_id));
}

TEST(InventoryState, LeftClickOnTheOffHandOrRingSlotRowUnequipsIt) {
    auto session = MakeTestSession();
    auto hero_id = MakeHero(session);
    auto shield_id =
        MakeCarriedEquippableItem(session.registry, hero_id, game_item::EquipSlot::OffHand);
    session.registry.Storage<component::NameComponent>().Add(shield_id, {.name = "Shield"});
    ASSERT_TRUE(game_item::Equip(session.registry, {.item = shield_id, .actor = hero_id},
                                 game_item::EquipSlot::OffHand));
    auto ring_id =
        MakeCarriedEquippableItem(session.registry, hero_id, game_item::EquipSlot::AnyRing);
    session.registry.Storage<component::NameComponent>().Add(ring_id, {.name = "Ring"});
    ASSERT_TRUE(game_item::Equip(session.registry, {.item = ring_id, .actor = hero_id},
                                 game_item::EquipSlot::RingLeft));
    game_states::InventoryState state(session);

    ui::RenderScene scene;
    state.BuildScene(scene);
    const auto [offhand_x, offhand_y] =
        RectCenter(ui::panel_layout::RowRect(ui::layout::kSidebarLowerPanelRect, 4));
    const auto offhand_widget = scene.HitTest(offhand_x, offhand_y);
    const auto [ring_x, ring_y] =
        RectCenter(ui::panel_layout::RowRect(ui::layout::kSidebarLowerPanelRect, 5));
    const auto ring_widget = scene.HitTest(ring_x, ring_y);

    if (!offhand_widget.has_value() || !ring_widget.has_value()) {
        FAIL() << "expected hits on the offhand and ring slot rows";
        return;
    }
    state.OnWidgetClick(offhand_widget.value(), ui::MouseButton::Left);
    state.OnWidgetClick(ring_widget.value(), ui::MouseButton::Left);

    EXPECT_FALSE(session.registry.Storage<game_item::EquippedComponent>().Has(shield_id));
    EXPECT_FALSE(session.registry.Storage<game_item::EquippedComponent>().Has(ring_id));
}

TEST(InventoryState, MiddleClickOnAnItemRowUsesItAndHealsTheHero) {
    auto session = MakeTestSession();
    auto hero_id = MakeHero(session);
    session.registry.Storage<component::HealthComponent>().Add(hero_id, {.current = 1, .max = 10});
    session.registry.Storage<component::NameComponent>().Add(hero_id, {.name = "Hero"});
    auto potion =
        MakeNamedMagicItem(session, hero_id, "Healing Potion", "healing_light", /*charges=*/1);
    game_states::InventoryState state(session);

    ui::RenderScene scene;
    state.BuildScene(scene);
    const auto [x, y] =
        RectCenter(ui::panel_layout::RowRect(ui::layout::kSidebarUpperPanelRect, 1));
    const auto widget = scene.HitTest(x, y);

    if (!widget.has_value()) {
        FAIL() << "expected a hit on the potion row";
        return;
    }
    state.OnWidgetClick(widget.value(), ui::MouseButton::Middle);

    EXPECT_EQ(session.registry.GetRef<component::HealthComponent>(hero_id).current, 5);
    EXPECT_FALSE(session.registry.Storage<game_item::OwnershipComponent>().Has(potion));

    EXPECT_TRUE(std::ranges::any_of(session.runtime.log, [](const std::string &line) {
        return line.contains("Healing Potion");
    }));
}

TEST(InventoryState, MiddleClickOnAnEquippedStaffCastsWithoutUnequipping) {
    auto session = MakeTestSession();
    auto hero_id = MakeHero(session);
    session.registry.Storage<component::HealthComponent>().Add(hero_id, {.current = 10, .max = 10});
    auto staff = MakeNamedMagicItem(session, hero_id, "Firestorm Staff", "fireball", /*charges=*/3);
    session.registry.Storage<game_item::EquippedComponent>().Add(
        staff, {.slot = game_item::EquipSlot::MainHand});
    game_states::InventoryState state(session);

    ui::RenderScene scene;
    state.BuildScene(scene);
    const auto [x, y] =
        RectCenter(ui::panel_layout::RowRect(ui::layout::kSidebarLowerPanelRect, 2));
    const auto widget = scene.HitTest(x, y);

    if (!widget.has_value()) {
        FAIL() << "expected a hit on the equipped staff row";
        return;
    }
    state.OnWidgetClick(widget.value(), ui::MouseButton::Middle);

    EXPECT_TRUE(session.registry.Storage<game_item::EquippedComponent>().Has(staff));
    ASSERT_TRUE(session.registry.Storage<game_item::ChargesComponent>().Has(staff));
    EXPECT_EQ(session.registry.Storage<game_item::ChargesComponent>().Get(staff)->amount, 2);
}

TEST(InventoryState, ClickOnAnEmptySlotRowDoesNothing) {
    auto session = MakeTestSession();
    MakeHero(session);
    game_states::InventoryState state(session);

    ui::RenderScene scene;
    state.BuildScene(scene);
    const auto [x, y] =
        RectCenter(ui::panel_layout::RowRect(ui::layout::kSidebarLowerPanelRect, 1));
    const auto widget = scene.HitTest(x, y);

    if (!widget.has_value()) {
        FAIL() << "expected a hit on the empty slot's inert region";
        return;
    }
    EXPECT_FALSE(widget.value().entity.has_value());
    EXPECT_NO_THROW(state.OnWidgetClick(widget.value(), ui::MouseButton::Left));
}

TEST(InventoryState, ClickOnTheTitleRowDoesNothing) {
    auto session = MakeTestSession();
    auto hero_id = MakeHero(session);
    auto item_id = MakeNamedEquippable(session, hero_id, "Sword");
    game_states::InventoryState state(session);

    ui::RenderScene scene;
    state.BuildScene(scene);
    const auto [x, y] =
        RectCenter(ui::panel_layout::RowRect(ui::layout::kSidebarUpperPanelRect, 0));
    const auto widget = scene.HitTest(x, y).value_or(ui::WidgetId{});

    EXPECT_FALSE(widget.entity.has_value());
    state.OnWidgetClick(widget, ui::MouseButton::Left);

    EXPECT_FALSE(session.registry.Storage<game_item::EquippedComponent>().Has(item_id));
}

TEST(InventoryState, ClickingCloseButtonPops) {
    auto session = MakeTestSession();
    MakeHero(session);
    game_states::InventoryState state(session);

    ui::RenderScene scene;
    state.BuildScene(scene);
    const auto [x, y] = RectCenter(ui::layout::kSidebarCloseButtonRect);
    const auto widget = scene.HitTest(x, y);

    if (!widget.has_value()) {
        FAIL() << "expected a hit on the close button";
        return;
    }
    auto result = state.OnWidgetClick(widget.value(), ui::MouseButton::Left);

    if (!result.has_value()) {
        FAIL() << "expected Close to produce a GameState::Result";
        return;
    }
    EXPECT_EQ(result.value().type, game_states::GameState::Result::Type::Pop);
}
