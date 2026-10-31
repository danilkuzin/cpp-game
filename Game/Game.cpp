#include "Game.h"
#include "Bindings.h"
#include "GameState.h"
#include "InputEvent.h"
#include "InputHandler.h"
#include "RenderScene.h"
#include "Renderer.h"
#include "StateStack.h"
#include <cassert>
#include <chrono>
#include <memory>
#include <thread>
#include <utility>
#include <variant>
#include <vector>

namespace game {

Game::Game(graphics::Renderer &renderer, std::unique_ptr<game_states::GameState> initial_state)
    : renderer_(renderer) {
    states_.push_back(std::move(initial_state));
}

void Game::Run() {
    auto quit = false;
    constexpr int kTargetFps = 30;
    constexpr std::chrono::duration<double> kFrameDuration{1.0 / kTargetFps};
    states_.back()->OnEnter();
    ui::RenderScene scene;

    while (!quit) {
        const auto frame_deadline = std::chrono::steady_clock::now() + kFrameDuration;
        renderer_.StartFrame();

        renderer_.SetTextInputActive(states_.back()->InputContext() == ui::InputContext::TextEntry);

        game_states::GameState::Result result{
            .type = game_states::GameState::Result::Type::Continue, .next_state = nullptr};

        for (const auto &event : graphics::InputHandler::ReadInput()) {
            result = states_.back()->HandleInput(event, scene);
            if (result.type != game_states::GameState::Result::Type::Continue) {
                break;
            }
            if (std::holds_alternative<ui::MouseMoveEvent>(event)) {
                continue;
            }
            result = states_.back()->Update();
            if (result.type != game_states::GameState::Result::Type::Continue) {
                break;
            }
        }

        game_states::ComposeScene(states_, scene);
        renderer_.Render(scene);

        renderer_.EndFrame();
        std::this_thread::sleep_until(frame_deadline);

        quit = game_states::ApplyTransition(states_, std::move(result));
    }
}
} // namespace game
