#include "Coordinates.h"
#include "Layout.h"
#include "RectInt.h"
#include "RenderScene.h"
#include "Vector2Int.h"
#include <cstdlib>
#include <optional>
#include <string>

namespace ui {
RenderSprite TileSprite(const std::string &path, Vector2Int tile, RectInt camera_view, int z_index,
                        bool dim) {
    const RectInt rect = TileRect(tile, camera_view);
    return {.x = rect.x,
            .y = rect.y,
            .width = rect.width,
            .height = rect.height,
            .sprite = path,
            .z_index = z_index,
            .dim = dim};
}

std::optional<Vector2Int> ScreenToTile(float x, float y, RectInt camera_view) {
    if (x < 0.F || y < 0.F) {
        return std::nullopt;
    }
    const int column = (static_cast<int>(x) / layout::kTileWidth) - layout::kWindowLeft;
    const int row = (static_cast<int>(y) / layout::kTileHeight) - layout::kWindowTop;
    if (column < 0 || row < 0) {
        return std::nullopt;
    }
    const Vector2Int tile{.x = camera_view.x + column, .y = camera_view.y + row};
    if (!camera_view.ContainsPoint(tile)) {
        return std::nullopt;
    }
    return tile;
}

std::optional<Vector2Int> CardinalToward(const Vector2Int origin, const Vector2Int tile) {
    if (tile == origin) {
        return std::nullopt;
    }
    const auto dcoord = tile - origin;
    if (std::abs(dcoord.y) > std::abs(dcoord.x)) {
        if (dcoord.y > 0) {
            return Vector2Int::Down();
        }
        return Vector2Int::Up();
    }
    if (dcoord.x > 0) {
        return Vector2Int::Right();
    }
    return Vector2Int::Left();
}
} // namespace ui
