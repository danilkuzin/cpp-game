#include "Bindings.h"
#include "Components.h"
#include "GameState.h"
#include "GameplayState.h"
#include "PanelRects.h"
#include "RenderScene.h"
#include "Session.h"
#include "Support/Fixtures.h"
#include "Support/MakeSession.h"
#include "Vector2Int.h"
#include "Widget.h"
#include <gtest/gtest.h>
#include <optional>

TEST(GameplayInput, MoveNorthStampsExactlyOneIntent) {
    auto session = MakeTestSession();
    const auto hero_id =
        MakeCreatureAt(session.registry, {.coord = {.x = 5, .y = 5}, .map_level = 0}, 10, false);
    game_states::GameplayState state(session);

    state.OnAction(ui::Action::MoveNorth);

    const auto movers = session.registry.View<component::IntentToMove>();
    ASSERT_EQ(movers.size(), 1U);
    EXPECT_EQ(movers[0], hero_id);
    EXPECT_EQ(session.registry.GetRef<component::IntentToMove>(movers[0]).dcoord, Vector2Int::Up());
}

TEST(GameplayInput, OpenInventoryPushesAndStampsNothing) {
    auto session = MakeTestSession();
    MakeCreatureAt(session.registry, {.coord = {.x = 5, .y = 5}, .map_level = 0}, 10, false);
    game_states::GameplayState state(session);

    auto result = state.OnAction(ui::Action::OpenInventory);

    if (!result.has_value()) {
        FAIL() << "expected OpenInventory to produce a GameState::Result";
        return;
    }
    EXPECT_EQ(result.value().type, game_states::GameState::Result::Type::Push);
    EXPECT_NE(result.value().next_state, nullptr);
    EXPECT_TRUE(session.registry.Storage<component::IntentToMove>().Empty());
}

TEST(GameplayInput, CancelStampsNothingAndReturnsNoResult) {
    auto session = MakeTestSession();
    MakeCreatureAt(session.registry, {.coord = {.x = 5, .y = 5}, .map_level = 0}, 10, false);
    game_states::GameplayState state(session);

    auto result = state.OnAction(ui::Action::Cancel);

    EXPECT_FALSE(result.has_value());
    EXPECT_TRUE(session.registry.Storage<component::IntentToMove>().Empty());
}

TEST(GameplayInput, CameraDoesNotMoveWhileTheHeroIsMidView) {
    auto session = MakeTestSession();
    MakeHeroAt(session.registry, {.x = 5, .y = 5});
    game_states::GameplayState state(session);
    const auto view_before = session.runtime.camera.View();

    state.OnAction(ui::Action::MoveEast);
    state.Update();

    const auto view_after = session.runtime.camera.View();
    EXPECT_EQ(view_after.x, view_before.x);
    EXPECT_EQ(view_after.y, view_before.y);
}

TEST(GameplayInput, CameraRecentresWhenTheHeroReachesTheScrollZone) {
    auto session = MakeTestSession();
    MakeHeroAt(session.registry, {.x = 6, .y = 5});
    game_states::GameplayState state(session);
    const auto view_before = session.runtime.camera.View();
    EXPECT_EQ(view_before.x, 4);
    EXPECT_EQ(view_before.y, 4);

    state.OnAction(ui::Action::MoveEast);
    state.Update();

    const auto view_after = session.runtime.camera.View();
    EXPECT_EQ(view_after.x, 5);
    EXPECT_EQ(view_after.y, 3);
}

TEST(GameplayInput, MouseMoveOverTheMapSetsTheHoverTile) {
    auto session = MakeTestSession();
    game_states::GameplayState state(session);

    state.OnMouseMove({.x = 125.F, .y = 125.F}, std::nullopt);

    const auto hover = state.HoverTile();
    ASSERT_TRUE(hover.has_value());
    EXPECT_EQ(hover.value().x, 5); // NOLINT(bugprone-unchecked-optional-access)
    EXPECT_EQ(hover.value().y, 5); // NOLINT(bugprone-unchecked-optional-access)
}

TEST(GameplayInput, MouseMoveOverTheHudClearsTheHoverTile) {
    auto session = MakeTestSession();
    game_states::GameplayState state(session);
    state.OnMouseMove({.x = 125.F, .y = 125.F}, std::nullopt);
    ASSERT_TRUE(state.HoverTile().has_value());

    state.OnMouseMove(
        {.x = 125.F, .y = 125.F},
        ui::WidgetId{.kind = ui::WidgetKind::None, .entity = std::nullopt, .action = std::nullopt});

    EXPECT_FALSE(state.HoverTile().has_value());
}

TEST(GameplayInput, MouseMoveOverTheWindowMarginClearsTheHoverTile) {
    auto session = MakeTestSession();
    game_states::GameplayState state(session);
    state.OnMouseMove({.x = 125.F, .y = 125.F}, std::nullopt);
    ASSERT_TRUE(state.HoverTile().has_value());

    state.OnMouseMove({.x = 10.F, .y = 10.F}, std::nullopt);

    EXPECT_FALSE(state.HoverTile().has_value());
}

TEST(GameplayInput, BuildScenePushesInertHitRegionsOverBothHudPanels) {
    auto session = MakeTestSession();
    MakeHeroAt(session.registry, {.x = 5, .y = 5});
    const game_states::GameplayState state(session);

    ui::RenderScene scene;
    state.BuildScene(scene);

    const auto info_hit =
        scene.HitTest(static_cast<float>(ui::layout::kGameplayInfoPanelRect.x) + 1.F,
                      static_cast<float>(ui::layout::kGameplayInfoPanelRect.y) + 1.F);
    const auto log_hit =
        scene.HitTest(static_cast<float>(ui::layout::kGameplayLogPanelRect.x) + 1.F,
                      static_cast<float>(ui::layout::kGameplayLogPanelRect.y) + 1.F);
    ASSERT_TRUE(info_hit.has_value());
    ASSERT_TRUE(log_hit.has_value());
    const auto info_kind = info_hit.value().kind; // NOLINT(bugprone-unchecked-optional-access)
    const auto log_kind = log_hit.value().kind;   // NOLINT(bugprone-unchecked-optional-access)
    EXPECT_EQ(info_kind, ui::WidgetKind::None);
    EXPECT_EQ(log_kind, ui::WidgetKind::None);
}
