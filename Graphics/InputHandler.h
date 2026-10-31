#pragma once

#include "InputEvent.h"
#include <SDL3/SDL.h> // IWYU pragma: keep
#include <optional>
#include <vector>

namespace graphics {

std::optional<ui::InputEvent> TranslateEvent(const SDL_Event &sdl_event);
ui::Key KeyFromSdl(SDL_Keycode key);

class InputHandler {
public:
    static std::vector<ui::InputEvent> ReadInput();
};
} // namespace graphics
