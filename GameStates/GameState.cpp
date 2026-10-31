#include "GameState.h"
#include "InputEvent.h"
#include "RenderScene.h"
#include <type_traits>
#include <variant>

namespace game_states {

GameState::Result GameState::HandleInput(const ui::InputEvent &event,
                                         const ui::RenderScene &last_scene) {
    GameState::Result state_result = {.type = GameState::Result::Type::Continue,
                                      .next_state = nullptr};

    std::visit(
        [&](auto &&e) {
            using T = std::decay_t<decltype(e)>;

            if constexpr (std::is_same_v<T, ui::KeyEvent>) {
                auto action = Resolve(InputContext(), e.key);
                if (action.has_value()) {
                    auto action_result = OnAction(action.value());
                    if (action_result.has_value()) {
                        state_result = std::move(action_result.value());
                    }
                }
            } else if constexpr (std::is_same_v<T, ui::TextInputEvent>) {
                OnTextInput(e);
            } else if constexpr (std::is_same_v<T, ui::MouseClickEvent>) {
                auto widget = last_scene.HitTest(e.x, e.y);
                if (widget.has_value()) {
                    auto action_result = OnWidgetClick(widget.value(), e.button);
                    if (action_result.has_value()) {
                        state_result = std::move(action_result.value());
                    }
                } else {
                    OnMouseClick(e);
                }
            } else if constexpr (std::is_same_v<T, ui::MouseMoveEvent>) {
                OnMouseMove(e, last_scene.HitTest(e.x, e.y));
            } else if constexpr (std::is_same_v<T, ui::QuitEvent>) {
                state_result = {.type = GameState::Result::Type::Quit, .next_state = nullptr};
            }
        },
        event);

    AfterInput();
    return state_result;
}
} // namespace game_states
