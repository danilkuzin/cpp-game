#include "PanelLayout.h"
#include "RectInt.h"
#include "Vector2Int.h"
#include <cstddef>
#include <optional>

namespace ui::panel_layout {
RectInt RowRect(RectInt panel_rect, std::size_t row_index) {
    return RectInt{.x = panel_rect.x,
                   .y = panel_rect.y + kPadding + (static_cast<int>(row_index) * kLineSpacing),
                   .width = panel_rect.width,
                   .height = kLineSpacing};
}

std::optional<std::size_t> HitTestRow(RectInt panel_rect, std::size_t row_count, float x, float y) {
    const Vector2Int point{.x = static_cast<int>(x), .y = static_cast<int>(y)};
    if (!panel_rect.ContainsPoint(point)) {
        return std::nullopt;
    }
    for (std::size_t row = 0; row < row_count; ++row) {
        if (RowRect(panel_rect, row).ContainsPoint(point)) {
            return row;
        }
    }
    return std::nullopt;
}
} // namespace ui::panel_layout
