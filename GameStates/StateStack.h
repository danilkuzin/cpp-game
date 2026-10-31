#pragma once

#include "GameState.h"
#include "RenderScene.h"
#include <memory>
#include <vector>

namespace game_states {
using StateStack = std::vector<std::unique_ptr<GameState>>;

bool ApplyTransition(StateStack &states, GameState::Result result);
void ComposeScene(const StateStack &states, ui::RenderScene &scene);
} // namespace game_states
