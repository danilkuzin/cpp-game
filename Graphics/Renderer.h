#pragma once

#include "Color.h"
#include "RectInt.h"
#include "RenderScene.h"
#include "SdlHandles.h"
#include "TextCache.h"
#include "TextureManager.h"
#include <SDL3/SDL.h> // IWYU pragma: keep
#include <string>

namespace graphics {
class Renderer {
public:
    Renderer();
    ~Renderer() = default;
    Renderer(const Renderer &) = delete;
    Renderer &operator=(const Renderer &) = delete;
    Renderer(Renderer &&) = delete;
    Renderer &operator=(Renderer &&) = delete;

    void StartFrame();
    void Render(const ui::RenderScene &scene);
    void EndFrame();

    void SetTextInputActive(bool active);

private:
    void WriteText(const std::string &txt, int x, int y, ui::Color text_color);
    void FillRect(const RectInt &rect, ui::Color color);

    SdlInit sdl_;
    TtfInit ttf_;
    WindowPtr window_;
    FontPtr font_;

    SDL_Surface *screen_surface_ = nullptr;
    const SDL_PixelFormatDetails *format_details_ = nullptr;

    TextureManager texture_manager_;
    TextCache text_cache_;
};
} // namespace graphics
