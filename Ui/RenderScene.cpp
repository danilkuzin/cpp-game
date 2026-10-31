#include "RenderScene.h"
#include "Vector2Int.h"
#include "Widget.h"
#include <optional>
#include <ranges>

namespace ui {
void RenderScene::Clear() {
    sprites.clear();
    texts.clear();
    panels.clear();
    buttons.clear();
    hit_regions.clear();
    outlines.clear();
}

std::optional<WidgetId> RenderScene::HitTest(float x, float y) const {
    const Vector2Int point{.x = static_cast<int>(x), .y = static_cast<int>(y)};
    for (const auto &region : std::ranges::views::reverse(hit_regions)) {
        if (region.rect.ContainsPoint(point)) {
            return region.id;
        }
    }
    return std::nullopt;
}
} // namespace ui
