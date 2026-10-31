#pragma once

#include <SDL3/SDL.h> // IWYU pragma: keep
#include <SDL3_ttf/SDL_ttf.h>
#include <memory>
#include <stdexcept>
#include <string>

namespace graphics {
class SDLException final : public std::runtime_error {
public:
    explicit SDLException(const std::string &message)
        : std::runtime_error(message + "\n" + SDL_GetError()) {}
};

class SdlInit {
public:
    explicit SdlInit(SDL_InitFlags flags) {
        if (!SDL_Init(flags)) {
            throw SDLException("SDL could not initialize");
        }
    }

    ~SdlInit() { SDL_Quit(); }

    SdlInit(const SdlInit &) = delete;
    SdlInit &operator=(const SdlInit &) = delete;
    SdlInit(SdlInit &&) = delete;
    SdlInit &operator=(SdlInit &&) = delete;
};

class TtfInit {
public:
    TtfInit() {
        if (!TTF_Init()) {
            throw SDLException("SDL_ttf could not initialize");
        }
    }

    ~TtfInit() { TTF_Quit(); }

    TtfInit(const TtfInit &) = delete;
    TtfInit &operator=(const TtfInit &) = delete;
    TtfInit(TtfInit &&) = delete;
    TtfInit &operator=(TtfInit &&) = delete;
};

struct WindowDeleter {
    void operator()(SDL_Window *window) const noexcept { SDL_DestroyWindow(window); }
};

using WindowPtr = std::unique_ptr<SDL_Window, WindowDeleter>;

struct FontDeleter {
    void operator()(TTF_Font *font) const noexcept { TTF_CloseFont(font); }
};

using FontPtr = std::unique_ptr<TTF_Font, FontDeleter>;
} // namespace graphics
