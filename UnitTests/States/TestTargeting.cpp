#include "Bindings.h"
#include "Components.h"
#include "GameplayState.h"
#include "InputEvent.h"
#include "PanelRects.h"
#include "Registry.h"
#include "RenderScene.h"
#include "Session.h"
#include "Support/Fixtures.h"
#include "Support/MakeSession.h"
#include "Vector2Int.h"
#include "Widget.h"
#include <gtest/gtest.h>
#include <optional>

TEST(Targeting, EnteringTargetingStampsNothing) {
    auto session = MakeTestSession();
    MakeHeroAt(session.registry, {.x = 5, .y = 5});
    game_states::GameplayState state(session);

    state.OnAction(ui::Action::CastSpell1);

    EXPECT_TRUE(session.registry.Storage<component::IntentToCastSpell>().Empty());
    EXPECT_TRUE(state.Targeting().has_value());
}

TEST(Targeting, ConfirmingStampsExactlyOneIntentWithTheChosenDirection) {
    auto session = MakeTestSession();
    MakeHeroAt(session.registry, {.x = 5, .y = 5});
    game_states::GameplayState state(session);

    state.OnAction(ui::Action::CastSpell1);
    state.OnAction(ui::Action::MoveEast);
    state.OnAction(ui::Action::Confirm);

    const auto casters = session.registry.View<component::IntentToCastSpell>();
    ASSERT_EQ(casters.size(), 1U);
    const auto &intent = session.registry.GetRef<component::IntentToCastSpell>(casters[0]);
    ASSERT_TRUE(intent.direction.has_value());
    const auto direction = intent.direction.value(); // NOLINT(bugprone-unchecked-optional-access)
    EXPECT_EQ(direction, Vector2Int::Right());
    EXPECT_EQ(intent.spell_id, "fireball");
    EXPECT_FALSE(state.Targeting().has_value());
}

TEST(Targeting, CancellingStampsNothing) {
    auto session = MakeTestSession();
    MakeHeroAt(session.registry, {.x = 5, .y = 5});
    game_states::GameplayState state(session);

    state.OnAction(ui::Action::CastSpell1);
    state.OnAction(ui::Action::Cancel);

    EXPECT_TRUE(session.registry.Storage<component::IntentToCastSpell>().Empty());
    EXPECT_FALSE(state.Targeting().has_value());
}

TEST(Targeting, MoveActionsAimInsteadOfMovingWhileTargeting) {
    auto session = MakeTestSession();
    MakeHeroAt(session.registry, {.x = 5, .y = 5});
    game_states::GameplayState state(session);
    state.OnAction(ui::Action::CastSpell1);

    state.OnAction(ui::Action::MoveEast);

    EXPECT_TRUE(session.registry.Storage<component::IntentToMove>().Empty());
    ASSERT_TRUE(state.Targeting().has_value());
    const auto direction =
        state.Targeting().value().direction; // NOLINT(bugprone-unchecked-optional-access)
    EXPECT_EQ(direction, Vector2Int::Right());
}

TEST(Targeting, MenuKeysAreSwallowedWhileTargeting) {
    auto session = MakeTestSession();
    MakeHeroAt(session.registry, {.x = 5, .y = 5});
    game_states::GameplayState state(session);
    state.OnAction(ui::Action::CastSpell1);

    auto result = state.OnAction(ui::Action::OpenInventory);

    EXPECT_FALSE(result.has_value());
    EXPECT_TRUE(state.Targeting().has_value());
}

TEST(Targeting, SwitchingSpellsStaysInTargeting) {
    auto session = MakeTestSession();
    MakeHeroAt(session.registry, {.x = 5, .y = 5});
    game_states::GameplayState state(session);
    state.OnAction(ui::Action::CastSpell1);

    state.OnAction(ui::Action::CastSpell1);

    ASSERT_TRUE(state.Targeting().has_value());
    const auto slot =
        state.Targeting().value().spell_slot; // NOLINT(bugprone-unchecked-optional-access)
    EXPECT_EQ(slot, 0U);
    EXPECT_TRUE(session.registry.Storage<component::IntentToCastSpell>().Empty());
}

TEST(Targeting, AHealingSpellCastsWithoutTargeting) {
    auto session = MakeTestSession();
    MakeHeroAt(session.registry, {.x = 5, .y = 5});
    game_states::GameplayState state(session);

    state.OnAction(ui::Action::CastSpell2);

    const auto casters = session.registry.View<component::IntentToCastSpell>();
    ASSERT_EQ(casters.size(), 1U);
    const auto &intent = session.registry.GetRef<component::IntentToCastSpell>(casters[0]);
    EXPECT_FALSE(intent.direction.has_value());
    EXPECT_EQ(intent.spell_id, "healing_light");
    EXPECT_FALSE(state.Targeting().has_value());
}

TEST(Targeting, IntentsDoNotSurviveATick) {
    auto session = MakeTestSession();
    MakeHeroAt(session.registry, {.x = 5, .y = 5});
    game_states::GameplayState state(session);
    state.OnAction(ui::Action::CastSpell1);
    state.OnAction(ui::Action::MoveEast);
    state.OnAction(ui::Action::Confirm);

    EXPECT_NO_THROW(state.Update());

    EXPECT_TRUE(session.registry.Storage<component::IntentToCastSpell>().Empty());
}

TEST(Targeting, ClickingATileConfirmsWithThatDirection) {
    auto session = MakeTestSession();
    MakeHeroAt(session.registry, {.x = 5, .y = 5});
    game_states::GameplayState state(session);
    state.OnAction(ui::Action::CastSpell1);

    state.OnMouseClick({.x = 175.F, .y = 125.F, .button = ui::MouseButton::Left});

    const auto casters = session.registry.View<component::IntentToCastSpell>();
    ASSERT_EQ(casters.size(), 1U);
    const auto &intent = session.registry.GetRef<component::IntentToCastSpell>(casters[0]);
    ASSERT_TRUE(intent.direction.has_value());
    const auto direction = intent.direction.value(); // NOLINT(bugprone-unchecked-optional-access)
    EXPECT_EQ(direction, Vector2Int::Right());
    EXPECT_FALSE(state.Targeting().has_value());
}

TEST(Targeting, ClickingTheWindowMarginDoesNotFire) {
    auto session = MakeTestSession();
    MakeHeroAt(session.registry, {.x = 5, .y = 5});
    game_states::GameplayState state(session);
    state.OnAction(ui::Action::CastSpell1);

    state.OnMouseClick({.x = 10.F, .y = 10.F, .button = ui::MouseButton::Left});

    EXPECT_TRUE(session.registry.Storage<component::IntentToCastSpell>().Empty());
    EXPECT_TRUE(state.Targeting().has_value());
}

TEST(Targeting, ClickingTheHerosOwnTileDoesNotFire) {
    auto session = MakeTestSession();
    MakeHeroAt(session.registry, {.x = 5, .y = 5});
    game_states::GameplayState state(session);
    state.OnAction(ui::Action::CastSpell1);

    state.OnMouseClick({.x = 125.F, .y = 125.F, .button = ui::MouseButton::Left});

    EXPECT_TRUE(session.registry.Storage<component::IntentToCastSpell>().Empty());
    EXPECT_TRUE(state.Targeting().has_value());
}

TEST(Targeting, MouseMoveAimsWhileTargeting) {
    auto session = MakeTestSession();
    MakeHeroAt(session.registry, {.x = 5, .y = 5});
    game_states::GameplayState state(session);
    state.OnAction(ui::Action::CastSpell1);

    state.OnMouseMove({.x = 125.F, .y = 75.F}, std::nullopt);

    ASSERT_TRUE(state.Targeting().has_value());
    const auto direction =
        state.Targeting().value().direction; // NOLINT(bugprone-unchecked-optional-access)
    EXPECT_EQ(direction, Vector2Int::Up());
    EXPECT_TRUE(session.registry.Storage<component::IntentToCastSpell>().Empty());
}

TEST(Targeting, MouseMoveOverTheHudKeepsTheLastAim) {
    auto session = MakeTestSession();
    MakeHeroAt(session.registry, {.x = 5, .y = 5});
    game_states::GameplayState state(session);
    state.OnAction(ui::Action::CastSpell1);
    state.OnMouseMove({.x = 175.F, .y = 125.F}, std::nullopt);

    state.OnMouseMove(
        {.x = 175.F, .y = 125.F},
        ui::WidgetId{.kind = ui::WidgetKind::None, .entity = std::nullopt, .action = std::nullopt});

    ASSERT_TRUE(state.Targeting().has_value());
    const auto direction =
        state.Targeting().value().direction; // NOLINT(bugprone-unchecked-optional-access)
    EXPECT_EQ(direction, Vector2Int::Right());
}

TEST(Targeting, ClickingASpellButtonEntersTargeting) {
    auto session = MakeTestSession();
    MakeHeroAt(session.registry, {.x = 5, .y = 5});
    game_states::GameplayState state(session);
    ui::RenderScene scene;
    state.BuildScene(scene);
    const auto [x, y] = RectCenter(ui::layout::SpellButtonRect(0));
    const auto widget = scene.HitTest(x, y);

    ASSERT_TRUE(widget.has_value());
    state.OnWidgetClick(widget.value(), // NOLINT(bugprone-unchecked-optional-access)
                        ui::MouseButton::Left);

    EXPECT_TRUE(session.registry.Storage<component::IntentToCastSpell>().Empty());
    EXPECT_TRUE(state.Targeting().has_value());
}

TEST(Targeting, ClickingTheHealingButtonCastsImmediately) {
    auto session = MakeTestSession();
    MakeHeroAt(session.registry, {.x = 5, .y = 5});
    game_states::GameplayState state(session);
    ui::RenderScene scene;
    state.BuildScene(scene);
    const auto [x, y] = RectCenter(ui::layout::SpellButtonRect(1));
    const auto widget = scene.HitTest(x, y);

    ASSERT_TRUE(widget.has_value());
    state.OnWidgetClick(widget.value(), // NOLINT(bugprone-unchecked-optional-access)
                        ui::MouseButton::Left);

    const auto casters = session.registry.View<component::IntentToCastSpell>();
    ASSERT_EQ(casters.size(), 1U);
    const auto &intent = session.registry.GetRef<component::IntentToCastSpell>(casters[0]);
    EXPECT_FALSE(intent.direction.has_value());
    EXPECT_EQ(intent.spell_id, "healing_light");
    EXPECT_FALSE(state.Targeting().has_value());
}
