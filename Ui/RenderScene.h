#pragma once

#include "Color.h"
#include "RectInt.h"
#include "Widget.h"
#include <optional>
#include <string>
#include <vector>

namespace ui {
struct RenderSprite {
    int x = 0, y = 0, width = 0, height = 0;
    std::string sprite;
    int z_index = 0;
    bool dim = false;
};

struct RenderText {
    std::string text;
    int x = 0, y = 0;
    ui::Color text_color;
};

struct RenderPanel {
    RectInt rect;
    ui::Color bg_color;
    ui::Color text_color;
    std::vector<std::string> rows;
};

struct RenderButton {
    RectInt rect;
    std::string label;
    ui::Color bg_color;
    ui::Color text_color;
};

struct RenderOutline {
    RectInt rect = {};
    ui::Color color = {};
    int thickness = 2;
};

struct RenderScene {
    std::vector<RenderSprite> sprites;
    std::vector<RenderText> texts;
    std::vector<RenderPanel> panels;
    std::vector<RenderButton> buttons;
    std::vector<HitRegion> hit_regions;
    std::vector<RenderOutline> outlines;
    [[nodiscard]] std::optional<WidgetId> HitTest(float x, float y) const;
    void Clear();
};
} // namespace ui
