#pragma once

#include "EntityStorage.h"
#include "RectInt.h"
#include "RenderScene.h"
#include "Widget.h"
#include <optional>
#include <span>
#include <string>

namespace ui {

struct ListRow {
    std::string text;
    std::optional<entity::EntityId> entity;
};

void PushSidebarBackdrop(RenderScene &scene);

void PushListPanel(RenderScene &scene, RectInt rect, const std::string &title,
                   std::span<const ListRow> rows, WidgetKind kind);

} // namespace ui
