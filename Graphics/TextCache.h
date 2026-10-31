#pragma once

#include "Color.h"
#include <SDL3/SDL.h> // IWYU pragma: keep
#include <SDL3_ttf/SDL_ttf.h>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace graphics {
class TextCache {
public:
    explicit TextCache(TTF_Font *font) : font_(font) {}

    ~TextCache();
    TextCache(const TextCache &) = delete;
    TextCache &operator=(const TextCache &) = delete;
    TextCache(TextCache &&) = delete;
    TextCache &operator=(TextCache &&) = delete;

    SDL_Surface *Get(const std::string &text, ui::Color color);

    void EndFrame();

private:
    static constexpr std::uint64_t kMaxIdleFrames = 120;

    struct Entry {
        ui::Color color;
        SDL_Surface *surface = nullptr;
        std::uint64_t last_used_frame = 0;
    };

    TTF_Font *font_;
    std::uint64_t frame_ = 0;
    std::unordered_map<std::string, std::vector<Entry>> cache_;
};
} // namespace graphics
