#include "SidebarPanel.h"
#include "PanelLayout.h"
#include "PanelRects.h"
#include "RectInt.h"
#include "RenderScene.h"
#include "Widget.h"
#include <optional>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace ui {

void PushSidebarBackdrop(RenderScene &scene) {
    scene.panels.push_back({.rect = ui::layout::kSidebarBackdropRect,
                            .bg_color = ui::layout::kPanelBgColor,
                            .text_color = ui::layout::kPanelTextColor,
                            .rows = {}});
    scene.hit_regions.push_back({.rect = ui::layout::kSidebarBackdropRect, .id = {}});
}

void PushListPanel(RenderScene &scene, RectInt rect, const std::string &title,
                   std::span<const ListRow> rows, WidgetKind kind) {
    std::vector<std::string> lines{title};
    for (const auto &row : rows) {
        lines.push_back(row.text);
        if (row.entity.has_value()) {
            scene.hit_regions.push_back(
                {.rect = ui::panel_layout::RowRect(rect, lines.size() - 1),
                 .id = {.kind = kind, .entity = row.entity, .action = std::nullopt}});
        }
    }
    scene.panels.push_back({.rect = rect,
                            .bg_color = ui::layout::kPanelBgColor,
                            .text_color = ui::layout::kPanelTextColor,
                            .rows = std::move(lines)});
}

} // namespace ui
