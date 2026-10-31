#include "Bindings.h"
#include "GameState.h"
#include "RenderScene.h"
#include "StateStack.h"
#include <algorithm>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace {
class DummyState : public game_states::GameState {
public:
    explicit DummyState(int &destroy_count, std::vector<std::string> &trace, std::string name)
        : destroy_count_(destroy_count), trace_(trace), name_(std::move(name)) {}

    ~DummyState() override { ++destroy_count_; }

    DummyState(const DummyState &) = delete;
    DummyState &operator=(const DummyState &) = delete;
    DummyState(DummyState &&) = delete;
    DummyState &operator=(DummyState &&) = delete;

    void OnEnter() override { trace_.push_back(name_ + ":enter"); }

    void OnExit() override { trace_.push_back(name_ + ":exit"); }

    [[nodiscard]] ui::InputContext InputContext() const override { return ui::InputContext::Menu; }

    void BuildScene(ui::RenderScene &scene) const override {
        scene.texts.push_back({.text = name_, .x = 0, .y = 0, .text_color = {}});
    }

    [[nodiscard]] bool IsOpaque() const override { return opaque; }

    bool opaque = true;

private:
    int &destroy_count_;
    std::vector<std::string> &trace_;
    std::string name_;
};

std::unique_ptr<DummyState> MakeState(int &destroy_count, std::vector<std::string> &trace,
                                      std::string name) {
    return std::make_unique<DummyState>(destroy_count, trace, std::move(name));
}

std::string TopName(const game_states::StateStack &states) {
    ui::RenderScene scene;
    states.back()->BuildScene(scene);
    return scene.texts.back().text;
}
} // namespace

TEST(StateStack, PushKeepsTheStateBelow) {
    int destroy_count = 0;
    std::vector<std::string> trace;
    game_states::StateStack states;
    states.push_back(MakeState(destroy_count, trace, "below"));

    game_states::ApplyTransition(states, {.type = game_states::GameState::Result::Type::Push,
                                          .next_state = MakeState(destroy_count, trace, "top")});

    ASSERT_EQ(states.size(), 2U);
    EXPECT_EQ(TopName(states), "top");
}

TEST(StateStack, PushDoesNotExitTheStateBelow) {
    int destroy_count = 0;
    std::vector<std::string> trace;
    game_states::StateStack states;
    states.push_back(MakeState(destroy_count, trace, "below"));
    trace.clear();

    game_states::ApplyTransition(states, {.type = game_states::GameState::Result::Type::Push,
                                          .next_state = MakeState(destroy_count, trace, "top")});

    EXPECT_EQ(std::ranges::find(trace, "below:exit"), trace.end());
}

TEST(StateStack, ReplaceDestroysTheOutgoingState) {
    int destroy_count = 0;
    std::vector<std::string> trace;
    game_states::StateStack states;
    states.push_back(MakeState(destroy_count, trace, "a"));

    game_states::ApplyTransition(states, {.type = game_states::GameState::Result::Type::Replace,
                                          .next_state = MakeState(destroy_count, trace, "b")});

    EXPECT_EQ(states.size(), 1U);
    EXPECT_EQ(destroy_count, 1);
}

TEST(StateStack, ReplaceExitsBeforeEnter) {
    int destroy_count = 0;
    std::vector<std::string> trace;
    game_states::StateStack states;
    states.push_back(MakeState(destroy_count, trace, "a"));
    trace.clear();

    game_states::ApplyTransition(states, {.type = game_states::GameState::Result::Type::Replace,
                                          .next_state = MakeState(destroy_count, trace, "b")});

    EXPECT_EQ(trace, (std::vector<std::string>{"a:exit", "b:enter"}));
}

TEST(StateStack, PopEntersTheRevealedState) {
    int destroy_count = 0;
    std::vector<std::string> trace;
    game_states::StateStack states;
    states.push_back(MakeState(destroy_count, trace, "below"));
    states.push_back(MakeState(destroy_count, trace, "top"));
    trace.clear();

    game_states::ApplyTransition(
        states, {.type = game_states::GameState::Result::Type::Pop, .next_state = nullptr});

    EXPECT_EQ(trace, (std::vector<std::string>{"top:exit", "below:enter"}));
}

TEST(StateStack, PopOnTheLastStateQuits) {
    int destroy_count = 0;
    std::vector<std::string> trace;
    game_states::StateStack states;
    states.push_back(MakeState(destroy_count, trace, "only"));

    const bool quit = game_states::ApplyTransition(
        states, {.type = game_states::GameState::Result::Type::Pop, .next_state = nullptr});

    EXPECT_TRUE(quit);
    EXPECT_TRUE(states.empty());
}

TEST(StateStack, QuitReturnsTrueAndLeavesTheStack) {
    int destroy_count = 0;
    std::vector<std::string> trace;
    game_states::StateStack states;
    states.push_back(MakeState(destroy_count, trace, "only"));

    const bool quit = game_states::ApplyTransition(
        states, {.type = game_states::GameState::Result::Type::Quit, .next_state = nullptr});

    EXPECT_TRUE(quit);
    EXPECT_EQ(states.size(), 1U);
}

TEST(StateStack, OpaqueTopStateBuildsAlone) {
    int destroy_count = 0;
    std::vector<std::string> trace;
    game_states::StateStack states;
    states.push_back(MakeState(destroy_count, trace, "below"));
    states.push_back(MakeState(destroy_count, trace, "top"));

    ui::RenderScene scene;
    game_states::ComposeScene(states, scene);

    ASSERT_EQ(scene.texts.size(), 1U);
    EXPECT_EQ(scene.texts[0].text, "top");
}

TEST(StateStack, NonOpaqueTopStateComposesTheStateBelow) {
    int destroy_count = 0;
    std::vector<std::string> trace;
    game_states::StateStack states;
    states.push_back(MakeState(destroy_count, trace, "below"));
    auto top = MakeState(destroy_count, trace, "top");
    top->opaque = false;
    states.push_back(std::move(top));

    ui::RenderScene scene;
    game_states::ComposeScene(states, scene);

    ASSERT_EQ(scene.texts.size(), 2U);
    EXPECT_EQ(scene.texts[0].text, "below");
    EXPECT_EQ(scene.texts[1].text, "top");
}

TEST(StateStack, TwoNonOpaqueStatesComposeAllThree) {
    int destroy_count = 0;
    std::vector<std::string> trace;
    game_states::StateStack states;
    states.push_back(MakeState(destroy_count, trace, "bottom"));
    auto middle = MakeState(destroy_count, trace, "middle");
    middle->opaque = false;
    states.push_back(std::move(middle));
    auto top = MakeState(destroy_count, trace, "top");
    top->opaque = false;
    states.push_back(std::move(top));

    ui::RenderScene scene;
    game_states::ComposeScene(states, scene);

    ASSERT_EQ(scene.texts.size(), 3U);
    EXPECT_EQ(scene.texts[0].text, "bottom");
    EXPECT_EQ(scene.texts[1].text, "middle");
    EXPECT_EQ(scene.texts[2].text, "top");
}

TEST(StateStack, TheBottomStateIsAlwaysDrawn) {
    int destroy_count = 0;
    std::vector<std::string> trace;
    game_states::StateStack states;
    auto only = MakeState(destroy_count, trace, "only");
    only->opaque = false;
    states.push_back(std::move(only));

    ui::RenderScene scene;
    game_states::ComposeScene(states, scene);

    ASSERT_EQ(scene.texts.size(), 1U);
    EXPECT_EQ(scene.texts[0].text, "only");
}

TEST(StateStack, ComposeClearsTheSceneFirst) {
    int destroy_count = 0;
    std::vector<std::string> trace;
    game_states::StateStack states;
    states.push_back(MakeState(destroy_count, trace, "only"));

    ui::RenderScene scene;
    scene.texts.push_back({.text = "stale", .x = 0, .y = 0, .text_color = {}});
    game_states::ComposeScene(states, scene);

    ASSERT_EQ(scene.texts.size(), 1U);
    EXPECT_EQ(scene.texts[0].text, "only");
}
