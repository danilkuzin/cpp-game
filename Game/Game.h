#pragma once

#include "GameState.h"
#include "Renderer.h"
#include <memory>
#include <vector>

namespace game {
class Game {

public:
    explicit Game(graphics::Renderer &renderer,
                  std::unique_ptr<game_states::GameState> initial_state);

    void Run();

private:
    std::vector<std::unique_ptr<game_states::GameState>> states_;
    graphics::Renderer &renderer_;
};
} // namespace game
