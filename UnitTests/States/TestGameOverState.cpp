#include "Bindings.h"
#include "GameOverState.h"
#include "GameState.h"
#include "RenderScene.h"
#include "Session.h"
#include "Support/MakeSession.h"
#include <gtest/gtest.h>
#include <string>
#include <vector>

namespace {
const std::vector<std::string> &SummaryRows(ui::RenderScene &scene,
                                            const game_states::GameOverState &state) {
    state.BuildScene(scene);
    EXPECT_EQ(scene.panels.size(), 1U);
    return scene.panels[0].rows;
}
} // namespace

TEST(GameOverState, SummaryRowsReportHeroNameLevelAndMonstersSlain) {
    auto session = MakeTestSession();
    session.stats = {
        .hero_name = "Bob", .level = 3, .monsters_slain = 7, .money = 0, .equipment = {}};
    const game_states::GameOverState state(session);

    ui::RenderScene scene;
    const auto &rows = SummaryRows(scene, state);

    ASSERT_EQ(rows.size(), 7U);
    EXPECT_EQ(rows[1], "Bob");
    EXPECT_NE(rows[2].find('3'), std::string::npos);
    EXPECT_NE(rows[3].find('7'), std::string::npos);
    EXPECT_EQ(rows.back(), "Press any key to quit");
}

TEST(GameOverState, EquipmentRowJoinsEquippedNamesAndReadsNothingWhenEmpty) {
    auto empty_session = MakeTestSession();
    empty_session.stats = {.hero_name = "Bob", .equipment = {}};
    const game_states::GameOverState empty_state(empty_session);
    ui::RenderScene empty_scene;
    EXPECT_NE(SummaryRows(empty_scene, empty_state)[5].find("(nothing)"), std::string::npos);

    auto session = MakeTestSession();
    session.stats = {.hero_name = "Bob", .equipment = {"Sword", "Helm"}};
    const game_states::GameOverState state(session);
    ui::RenderScene scene;
    const auto &row = SummaryRows(scene, state)[5];
    EXPECT_NE(row.find("Sword"), std::string::npos);
    EXPECT_NE(row.find("Helm"), std::string::npos);
}

TEST(GameOverState, IsNotOpaque) {
    auto session = MakeTestSession();
    const game_states::GameOverState state(session);

    EXPECT_FALSE(state.IsOpaque());
}

TEST(GameOverState, ConfirmQuits) {
    auto session = MakeTestSession();
    game_states::GameOverState state(session);

    auto result = state.OnAction(ui::Action::Confirm);

    if (!result.has_value()) {
        FAIL() << "expected Confirm to produce a GameState::Result";
        return;
    }
    EXPECT_EQ(result.value().type, game_states::GameState::Result::Type::Quit);
}

TEST(GameOverState, CancelQuits) {
    auto session = MakeTestSession();
    game_states::GameOverState state(session);

    auto result = state.OnAction(ui::Action::Cancel);

    if (!result.has_value()) {
        FAIL() << "expected Cancel to produce a GameState::Result";
        return;
    }
    EXPECT_EQ(result.value().type, game_states::GameState::Result::Type::Quit);
}
