#include "Renderer.h"
#include "Color.h"
#include "Coordinates.h"
#include "Layout.h"
#include "PanelLayout.h"
#include "RectInt.h"
#include "RenderScene.h"
#include "SdlHandles.h"
#include <SDL3/SDL.h> // IWYU pragma: keep
#include <SDL3_ttf/SDL_ttf.h>
#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>

namespace graphics {
namespace {
constexpr ui::Color kStartupColor{.r = 0xAA, .g = 0xAA, .b = 0xAA};
constexpr ui::Color kFrameClearColor{.r = 0xFF, .g = 0xFF, .b = 0xFF};

constexpr RectInt kScreenRect{
    .x = 0, .y = 0, .width = ui::layout::kScreenWidth, .height = ui::layout::kScreenHeight};

constexpr Uint8 kDimLevel = 90;
constexpr Uint8 kFullBrightness = 255;

constexpr int kFontSize = 16;
} // namespace

Renderer::Renderer()
    : sdl_(SDL_INIT_VIDEO),
      window_(SDL_CreateWindow("CPP Game", ui::layout::kScreenWidth, ui::layout::kScreenHeight, 0)),
      font_(TTF_OpenFont("Roboto-Regular.ttf", kFontSize)), text_cache_(font_.get()) {
    if (window_ == nullptr) {
        throw SDLException("Window could not be created");
    }
    if (font_ == nullptr) {
        throw SDLException("Error loading font");
    }

    screen_surface_ = SDL_GetWindowSurface(window_.get());
    if (screen_surface_ == nullptr) {
        throw SDLException("Window surface could not be created");
    }
    format_details_ = SDL_GetPixelFormatDetails(screen_surface_->format);
    if (format_details_ == nullptr) {
        throw SDLException("Window surface has no pixel format");
    }

    FillRect(kScreenRect, kStartupColor);
    if (!SDL_UpdateWindowSurface(window_.get())) {
        throw SDLException("Window surface could not be updated");
    }
}

void Renderer::SetTextInputActive(bool active) {
    if (active) {
        SDL_StartTextInput(window_.get());
    } else {
        SDL_StopTextInput(window_.get());
    }
}

void Renderer::FillRect(const RectInt &rect, ui::Color color) {
    const SDL_Rect sdl_rect = {.x = rect.x, .y = rect.y, .w = rect.width, .h = rect.height};
    SDL_FillSurfaceRect(screen_surface_, &sdl_rect,
                        SDL_MapRGB(format_details_, nullptr, color.r, color.g, color.b));
}

void Renderer::WriteText(const std::string &txt, int x, int y, ui::Color text_color) {
    SDL_Surface *text_surface = text_cache_.Get(txt, text_color);
    if (text_surface == nullptr) {
        return;
    }
    const SDL_Rect dst = {.x = x, .y = y, .w = text_surface->w, .h = text_surface->h};
    SDL_BlitSurface(text_surface, nullptr, screen_surface_, &dst);
}

void Renderer::StartFrame() { FillRect(kScreenRect, kFrameClearColor); }

void Renderer::EndFrame() {
    SDL_UpdateWindowSurface(window_.get());
    text_cache_.EndFrame();
}

void Renderer::Render(const ui::RenderScene &scene) {
    std::vector<const ui::RenderSprite *> ordered;
    ordered.reserve(scene.sprites.size());
    for (const auto &sprite : scene.sprites) {
        ordered.push_back(&sprite);
    }
    std::ranges::stable_sort(ordered, [](const ui::RenderSprite *a, const ui::RenderSprite *b) {
        return a->z_index < b->z_index;
    });

    for (const auto *sprt : ordered) {
        SDL_Surface *surface =
            texture_manager_.Get(sprt->sprite, {.x = sprt->width, .y = sprt->height});
        if (surface == nullptr) {
            continue;
        }

        const Uint8 mod = sprt->dim ? kDimLevel : kFullBrightness;
        SDL_SetSurfaceColorMod(surface, mod, mod, mod);

        const SDL_Rect dst = {.x = sprt->x, .y = sprt->y, .w = sprt->width, .h = sprt->height};
        SDL_BlitSurface(surface, nullptr, screen_surface_, &dst);
    }

    for (const auto &outline : scene.outlines) {
        for (const RectInt &edge : ui::OutlineEdges(outline.rect, outline.thickness)) {
            FillRect(edge, outline.color);
        }
    }

    for (const auto &txt : scene.texts) {
        WriteText(txt.text, txt.x, txt.y, txt.text_color);
    }

    for (const auto &panel : scene.panels) {
        FillRect(panel.rect, panel.bg_color);
        for (std::size_t i = 0; i < panel.rows.size(); ++i) {
            const RectInt row_rect = ui::panel_layout::RowRect(panel.rect, i);
            WriteText(panel.rows[i], row_rect.x + ui::panel_layout::kPadding, row_rect.y,
                      panel.text_color);
        }
    }

    for (const auto &button : scene.buttons) {
        FillRect(button.rect, button.bg_color);
        const RectInt label_rect = ui::panel_layout::RowRect(button.rect, 0);
        WriteText(button.label, label_rect.x + ui::panel_layout::kPadding, label_rect.y,
                  button.text_color);
    }
}
} // namespace graphics
