#pragma once

#include <cstdint>
#include <memory>
#include <optional>

#include "Bindings.h"
#include "InputEvent.h"
#include "RenderScene.h"
#include "Widget.h"

namespace game_states {

class GameState {
public:
    struct Result {
        enum class Type : std::uint8_t { Continue, Push, Replace, Pop, Quit };

        Type type = Type::Continue;
        std::unique_ptr<GameState> next_state;
    };

    GameState() = default;
    virtual ~GameState() = default;
    GameState(const GameState &) = delete;
    GameState &operator=(const GameState &) = delete;
    GameState(GameState &&) = delete;
    GameState &operator=(GameState &&) = delete;

    virtual void OnEnter() {}

    virtual void OnExit() {}

    Result HandleInput(const ui::InputEvent &event, const ui::RenderScene &last_scene);

    Result HandleInput(const ui::InputEvent &event) {
        return HandleInput(event, ui::RenderScene{});
    }

    virtual std::optional<Result> OnWidgetClick([[maybe_unused]] ui::WidgetId widget,
                                                [[maybe_unused]] ui::MouseButton button) {
        return {};
    }

    virtual Result Update() { return {}; }

    virtual void BuildScene(ui::RenderScene &scene) const = 0;

    virtual bool IsOpaque() const { return true; }

    virtual ui::InputContext InputContext() const = 0;

    virtual std::optional<Result> OnAction([[maybe_unused]] ui::Action action) {
        return std::nullopt;
    };

    virtual void OnMouseClick([[maybe_unused]] ui::MouseClickEvent event) {};

    virtual void OnMouseMove([[maybe_unused]] ui::MouseMoveEvent event,
                             [[maybe_unused]] std::optional<ui::WidgetId> widget) {}

    virtual void OnTextInput(ui::TextInputEvent){};
    virtual void AfterInput() {};
};

} // namespace game_states
