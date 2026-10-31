#include "TextureManager.h"
#include "Vector2Int.h"
#include <SDL3/SDL.h> // IWYU pragma: keep
#include <string>

namespace graphics {
namespace {
constexpr Uint8 kColorKeyR = 255;
constexpr Uint8 kColorKeyG = 0;
constexpr Uint8 kColorKeyB = 255;

constexpr SDL_PixelFormat kSurfaceFormat = SDL_PIXELFORMAT_XRGB8888;
} // namespace

TextureManager::~TextureManager() {
    for (auto &[_, entry] : cache_) {
        for (auto &scaled : entry.scaled) {
            SDL_DestroySurface(scaled.surface);
        }
        SDL_DestroySurface(entry.source);
    }
}

SDL_Surface *TextureManager::Get(const std::string &path, Vector2Int size) {
    auto [it, inserted] = cache_.try_emplace(path);
    Entry &entry = it->second;

    if (inserted) {
        SDL_Surface *raw = SDL_LoadBMP(path.c_str());
        if (raw == nullptr) {
            SDL_Log("Error loading sprite: %s. ", path.c_str());
            return nullptr;
        }
        entry.source = SDL_ConvertSurface(raw, kSurfaceFormat);
        SDL_DestroySurface(raw);
        if (entry.source == nullptr) {
            SDL_Log("Error converting sprite: %s. ", path.c_str());
            return nullptr;
        }
    }
    if (entry.source == nullptr) {
        return nullptr;
    }

    for (const auto &scaled : entry.scaled) {
        if (scaled.size == size) {
            return scaled.surface;
        }
    }

    SDL_Surface *scaled = SDL_CreateSurface(size.x, size.y, kSurfaceFormat);
    if (scaled == nullptr) {
        SDL_Log("Error allocating scaled sprite: %s. ", path.c_str());
        return nullptr;
    }
    // The source carries no colour key, so the magenta survives the scale as ordinary pixels;
    // nearest-neighbour keeps it exactly magenta. The key then goes on the copy that gets blitted.
    SDL_BlitSurfaceScaled(entry.source, nullptr, scaled, nullptr, SDL_SCALEMODE_NEAREST);
    SDL_SetSurfaceColorKey(scaled, true,
                           SDL_MapSurfaceRGB(scaled, kColorKeyR, kColorKeyG, kColorKeyB));

    entry.scaled.push_back({.size = size, .surface = scaled});
    return scaled;
}
} // namespace graphics
