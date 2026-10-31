#include "TextCache.h"
#include "Color.h"
#include <SDL3/SDL.h> // IWYU pragma: keep
#include <SDL3_ttf/SDL_ttf.h>
#include <iterator>
#include <string>
#include <vector>

namespace graphics {
namespace {
SDL_Color ToSdl(ui::Color in) { return {.r = in.r, .g = in.g, .b = in.b, .a = SDL_ALPHA_OPAQUE}; }
} // namespace

TextCache::~TextCache() {
    for (auto &[_, entries] : cache_) {
        for (auto &entry : entries) {
            SDL_DestroySurface(entry.surface);
        }
    }
}

SDL_Surface *TextCache::Get(const std::string &text, ui::Color color) {
    auto &entries = cache_[text];
    for (auto &entry : entries) {
        if (entry.color == color) {
            entry.last_used_frame = frame_;
            return entry.surface;
        }
    }

    SDL_Surface *surface = TTF_RenderText_Solid(font_, text.c_str(), 0, ToSdl(color));
    if (surface == nullptr) {
        SDL_Log("Error rendering text: %s. ", text.c_str());
        return nullptr;
    }
    entries.push_back({.color = color, .surface = surface, .last_used_frame = frame_});
    return surface;
}

void TextCache::EndFrame() {
    ++frame_;
    for (auto it = cache_.begin(); it != cache_.end();) {
        auto &entries = it->second;
        std::erase_if(entries, [this](const Entry &entry) {
            if (frame_ - entry.last_used_frame <= kMaxIdleFrames) {
                return false;
            }
            SDL_DestroySurface(entry.surface);
            return true;
        });
        it = entries.empty() ? cache_.erase(it) : std::next(it);
    }
}
} // namespace graphics
