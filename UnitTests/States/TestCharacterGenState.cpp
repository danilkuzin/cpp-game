#include "Bindings.h"
#include "CharacterGenState.h"
#include "Color.h"
#include "GameState.h"
#include "InputEvent.h"
#include "Location.h"
#include "RegistryQueries.h"
#include "RenderScene.h"
#include "Session.h"
#include "Support/MakeSession.h"
#include <gtest/gtest.h>

namespace {
ui::InputEvent Key(ui::Key key) { return ui::KeyEvent{.key = key}; }

} // namespace

TEST(CharacterGenState, StartsOnChoosingClass) {
    auto session = MakeTestSession();
    const game_states::CharacterGenState state(session);

    ui::RenderScene scene;
    state.BuildScene(scene);

    ASSERT_FALSE(scene.texts.empty());
    EXPECT_EQ(scene.texts[0].text, "Character Creation");
    ASSERT_GE(scene.texts.size(), 2U);
    EXPECT_EQ(scene.texts[1].text, "Choose your class:");
}

TEST(CharacterGenState, ReturnAdvancesThroughStages) {
    auto session = MakeTestSession();
    game_states::CharacterGenState state(session);

    state.HandleInput(Key(ui::Key::Return));
    ui::RenderScene race_scene;
    state.BuildScene(race_scene);
    EXPECT_EQ(race_scene.texts[1].text, "Choose your race:");

    state.HandleInput(Key(ui::Key::Return));
    EXPECT_EQ(state.InputContext(), ui::InputContext::TextEntry);
    ui::RenderScene name_scene;
    state.BuildScene(name_scene);
    EXPECT_EQ(name_scene.texts[1].text, "Enter your name:");
}

TEST(CharacterGenState, TypingEntersName) {
    auto session = MakeTestSession();
    game_states::CharacterGenState state(session);
    state.HandleInput(Key(ui::Key::Return));
    state.HandleInput(Key(ui::Key::Return));

    state.HandleInput(ui::TextInputEvent{.text = "a"});
    state.HandleInput(ui::TextInputEvent{.text = "b"});
    state.HandleInput(Key(ui::Key::Backspace));

    ui::RenderScene scene;
    state.BuildScene(scene);
    EXPECT_EQ(scene.texts.back().text, "a");
}

TEST(CharacterGenState, NextOptionCyclesClassIndexForwardAndWrapsAround) {
    auto session = MakeTestSession();
    game_states::CharacterGenState state(session);

    state.HandleInput(Key(ui::Key::Down));

    ui::RenderScene scene;
    state.BuildScene(scene);
    ASSERT_GE(scene.texts.size(), 5U);
    EXPECT_EQ(scene.texts[2].text_color, ui::kBlack);
    EXPECT_EQ(scene.texts[3].text_color, ui::kYellow);
    EXPECT_EQ(scene.texts[4].text_color, ui::kBlack);

    state.HandleInput(Key(ui::Key::Down));
    state.HandleInput(Key(ui::Key::Down));

    ui::RenderScene wrapped_scene;
    state.BuildScene(wrapped_scene);
    EXPECT_EQ(wrapped_scene.texts[2].text_color, ui::kYellow);
}

TEST(CharacterGenState, PrevOptionWrapsClassIndexBackwardPastZero) {
    auto session = MakeTestSession();
    game_states::CharacterGenState state(session);

    state.HandleInput(Key(ui::Key::Up));

    ui::RenderScene scene;
    state.BuildScene(scene);
    ASSERT_GE(scene.texts.size(), 5U);
    EXPECT_EQ(scene.texts[4].text_color, ui::kYellow);
}

TEST(CharacterGenState, NextOptionCyclesRaceIndexOnTheRaceStage) {
    auto session = MakeTestSession();
    game_states::CharacterGenState state(session);
    state.HandleInput(Key(ui::Key::Return));

    state.HandleInput(Key(ui::Key::Down));

    ui::RenderScene scene;
    state.BuildScene(scene);
    ASSERT_GE(scene.texts.size(), 6U);
    EXPECT_EQ(scene.texts[2].text_color, ui::kBlack);
    EXPECT_EQ(scene.texts[3].text_color, ui::kYellow);
}

TEST(CharacterGenState, FinishingCreatesHeroAndTransitionsToGameplay) {
    auto session = MakeTestSession();
    game_states::CharacterGenState state(session);
    state.HandleInput(Key(ui::Key::Return));
    state.HandleInput(Key(ui::Key::Return));
    state.HandleInput(ui::TextInputEvent{.text = "Hero"});
    state.HandleInput(Key(ui::Key::Return));

    auto result = state.Update();

    EXPECT_EQ(result.type, game_states::GameState::Result::Type::Replace);
    ASSERT_NE(result.next_state, nullptr);
    const auto hero_id = systems::PlayerId(session.registry);
    ASSERT_TRUE(hero_id.has_value());
    EXPECT_TRUE(session.registry.HasAll<Location>(
        hero_id.value())); // NOLINT(bugprone-unchecked-optional-access)
}

TEST(CharacterGenState, QuitEventReturnsQuit) {
    auto session = MakeTestSession();
    game_states::CharacterGenState state(session);
    auto result = state.HandleInput(ui::QuitEvent{});
    EXPECT_EQ(result.type, game_states::GameState::Result::Type::Quit);
}
