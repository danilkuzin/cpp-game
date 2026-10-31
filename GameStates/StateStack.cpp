#include "StateStack.h"
#include "GameState.h"
#include "RenderScene.h"
#include <cassert>
#include <cstddef>
#include <utility>

namespace game_states {
namespace {
void EnterTopState(StateStack &states) { states.back()->OnEnter(); }
} // namespace

bool ApplyTransition(StateStack &states, GameState::Result result) {
    auto quit = false;

    switch (result.type) {
    case game_states::GameState::Result::Type::Push:
        assert(result.next_state != nullptr);
        states.push_back(std::move(result.next_state));
        EnterTopState(states);
        break;

    case game_states::GameState::Result::Type::Replace:
        assert(result.next_state != nullptr);
        assert(!states.empty());
        states.back()->OnExit();
        states.pop_back();
        states.push_back(std::move(result.next_state));
        EnterTopState(states);
        break;

    case game_states::GameState::Result::Type::Pop:
        assert(result.next_state == nullptr);
        assert(!states.empty());
        states.back()->OnExit();
        states.pop_back();
        if (states.empty()) {
            quit = true;
        } else {
            EnterTopState(states);
        }
        break;

    case game_states::GameState::Result::Type::Quit:
        assert(result.next_state == nullptr);
        quit = true;
        break;

    default:
        break;
    }

    return quit;
}

void ComposeScene(const StateStack &states, ui::RenderScene &scene) {
    assert(!states.empty());
    scene.Clear();
    std::size_t bottom = states.size() - 1;
    while (bottom > 0 && !states[bottom]->IsOpaque()) {
        --bottom;
    }
    for (std::size_t i = bottom; i < states.size(); ++i) {
        states[i]->BuildScene(scene);
    }
}
} // namespace game_states
