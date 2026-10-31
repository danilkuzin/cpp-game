#pragma once

#include "Bindings.h"
#include "EntityStorage.h"
#include "RectInt.h"
#include <cstdint>
#include <optional>

namespace ui {

enum class WidgetKind : std::uint8_t {
    None,
    UpperRow,
    LowerRow,
    CloseButton,
    SpellButton,
};

struct WidgetId {
    WidgetKind kind = WidgetKind::None;
    std::optional<entity::EntityId> entity;
    std::optional<ui::Action> action;
};

struct HitRegion {
    RectInt rect{};
    WidgetId id;
};
} // namespace ui
