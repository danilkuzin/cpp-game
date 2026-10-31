#pragma once

#include "RectInt.h"
#include <cstddef>
#include <optional>

namespace ui::panel_layout {
constexpr int kPadding = 5;
constexpr int kLineSpacing = 18;

RectInt RowRect(RectInt panel_rect, std::size_t row_index);

constexpr int PanelHeight(std::size_t row_count) {
    return (2 * kPadding) + (static_cast<int>(row_count) * kLineSpacing);
}

std::optional<std::size_t> HitTestRow(RectInt panel_rect, std::size_t row_count, float x, float y);
} // namespace ui::panel_layout
