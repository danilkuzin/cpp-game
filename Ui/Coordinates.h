#pragma once

#include "Layout.h"
#include "RectInt.h"
#include "RenderScene.h"
#include "Vector2Int.h"
#include <array>
#include <optional>
#include <string>

namespace ui {
constexpr Vector2Int TileToScreen(Vector2Int tile, RectInt camera_view) {
    return {.x = (layout::kWindowLeft + tile.x - camera_view.x) * layout::kTileWidth,
            .y = (layout::kWindowTop + tile.y - camera_view.y) * layout::kTileHeight};
}

constexpr RectInt TileRect(Vector2Int tile, RectInt camera_view) {
    const Vector2Int top_left = TileToScreen(tile, camera_view);
    return {.x = top_left.x,
            .y = top_left.y,
            .width = layout::kTileWidth,
            .height = layout::kTileHeight};
}

constexpr std::array<RectInt, 4> OutlineEdges(RectInt rect, int thickness) {
    return {RectInt{.x = rect.x, .y = rect.y, .width = rect.width, .height = thickness},
            RectInt{.x = rect.x + rect.width - thickness,
                    .y = rect.y,
                    .width = thickness,
                    .height = rect.height},
            RectInt{.x = rect.x,
                    .y = rect.y + rect.height - thickness,
                    .width = rect.width,
                    .height = thickness},
            RectInt{.x = rect.x, .y = rect.y, .width = thickness, .height = rect.height}};
}

RenderSprite TileSprite(const std::string &path, Vector2Int tile, RectInt camera_view, int z_index,
                        bool dim = false);

std::optional<Vector2Int> ScreenToTile(float x, float y, RectInt camera_view);

std::optional<Vector2Int> CardinalToward(Vector2Int origin, Vector2Int tile);
} // namespace ui
