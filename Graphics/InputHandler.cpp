#include "InputHandler.h"
#include "InputEvent.h"
#include <SDL3/SDL.h> // IWYU pragma: keep
#include <optional>
#include <string>
#include <vector>

namespace graphics {

ui::Key KeyFromSdl(SDL_Keycode key) {
    switch (key) {
    case SDLK_UP:
        return ui::Key::Up;
    case SDLK_DOWN:
        return ui::Key::Down;
    case SDLK_LEFT:
        return ui::Key::Left;
    case SDLK_RIGHT:
        return ui::Key::Right;
    case SDLK_RETURN:
        return ui::Key::Return;
    case SDLK_ESCAPE:
        return ui::Key::Escape;
    case SDLK_BACKSPACE:
        return ui::Key::Backspace;
    case SDLK_SPACE:
        return ui::Key::Space;
    case SDLK_A:
        return ui::Key::A;
    case SDLK_D:
        return ui::Key::D;
    case SDLK_H:
        return ui::Key::H;
    case SDLK_I:
        return ui::Key::I;
    case SDLK_P:
        return ui::Key::P;
    case SDLK_W:
        return ui::Key::W;
    case SDLK_Z:
        return ui::Key::Z;
    case SDLK_1:
        return ui::Key::Num1;
    case SDLK_2:
        return ui::Key::Num2;
    default:
        return ui::Key::Unknown;
    }
}

namespace {

ui::MouseButton MouseButtonFromSdl(Uint8 button) {
    switch (button) {
    case SDL_BUTTON_LEFT:
        return ui::MouseButton::Left;
    case SDL_BUTTON_MIDDLE:
        return ui::MouseButton::Middle;
    case SDL_BUTTON_RIGHT:
        return ui::MouseButton::Right;
    default:
        return ui::MouseButton::Unknown;
    }
}

} // namespace

std::optional<ui::InputEvent> TranslateEvent(const SDL_Event &sdl_event) {
    switch (sdl_event.type) {
    case SDL_EVENT_TEXT_INPUT:
        return ui::TextInputEvent{.text = std::string(sdl_event.text.text)};

    case SDL_EVENT_KEY_DOWN:
        return ui::KeyEvent{.key = KeyFromSdl(sdl_event.key.key)};

    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        return ui::MouseClickEvent{.x = sdl_event.button.x,
                                   .y = sdl_event.button.y,
                                   .button = MouseButtonFromSdl(sdl_event.button.button)};

    case SDL_EVENT_MOUSE_MOTION:
        return ui::MouseMoveEvent{.x = sdl_event.motion.x, .y = sdl_event.motion.y};

    case SDL_EVENT_QUIT:
        return ui::QuitEvent();

    default:
        return std::nullopt;
    }
}

std::vector<ui::InputEvent> InputHandler::ReadInput() {
    std::vector<ui::InputEvent> res;
    SDL_Event e;

    while (SDL_PollEvent(&e)) {
        auto translated_event = TranslateEvent(e);
        if (translated_event.has_value()) {
            res.emplace_back(translated_event.value());
        }
    }
    return res;
}

} // namespace graphics
