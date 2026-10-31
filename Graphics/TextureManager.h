#pragma once

#include "Vector2Int.h"
#include <SDL3/SDL.h> // IWYU pragma: keep
#include <string>
#include <unordered_map>
#include <vector>

namespace graphics {
class TextureManager {
public:
    TextureManager() = default;
    ~TextureManager();
    TextureManager(const TextureManager &) = delete;
    TextureManager &operator=(const TextureManager &) = delete;
    TextureManager(TextureManager &&) = delete;
    TextureManager &operator=(TextureManager &&) = delete;

    SDL_Surface *Get(const std::string &path, Vector2Int size);

private:
    struct Scaled {
        Vector2Int size;
        SDL_Surface *surface = nullptr;
    };

    struct Entry {
        SDL_Surface *source = nullptr;
        std::vector<Scaled> scaled;
    };

    std::unordered_map<std::string, Entry> cache_;
};
} // namespace graphics
