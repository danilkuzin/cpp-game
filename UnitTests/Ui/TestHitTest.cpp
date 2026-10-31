#include "RectInt.h"
#include "RenderScene.h"
#include "Widget.h"
#include <gtest/gtest.h>
#include <optional>

namespace {
ui::HitRegion MakeRegion(RectInt rect, ui::WidgetId id) { return {.rect = rect, .id = id}; }
} // namespace

TEST(HitTest, HitsTheRowItLandsIn) {
    ui::RenderScene scene;
    scene.hit_regions.push_back(
        MakeRegion(RectInt{.x = 0, .y = 0, .width = 10, .height = 10},
                   {.kind = ui::WidgetKind::UpperRow, .entity = 1, .action = std::nullopt}));
    scene.hit_regions.push_back(
        MakeRegion(RectInt{.x = 0, .y = 10, .width = 10, .height = 10},
                   {.kind = ui::WidgetKind::UpperRow, .entity = 2, .action = std::nullopt}));

    const auto hit = scene.HitTest(5, 15);

    if (!hit.has_value()) {
        FAIL() << "expected a hit";
        return;
    }
    EXPECT_EQ(hit.value().kind, ui::WidgetKind::UpperRow);
    EXPECT_EQ(hit.value().entity, 2U);
}

TEST(HitTest, HitsAButton) {
    ui::RenderScene scene;
    scene.hit_regions.push_back(MakeRegion(
        RectInt{.x = 0, .y = 0, .width = 10, .height = 10},
        {.kind = ui::WidgetKind::CloseButton, .entity = std::nullopt, .action = std::nullopt}));

    const auto hit = scene.HitTest(5, 5);

    if (!hit.has_value()) {
        FAIL() << "expected a hit";
        return;
    }
    EXPECT_EQ(hit.value().kind, ui::WidgetKind::CloseButton);
    EXPECT_FALSE(hit.value().entity.has_value());
}

TEST(HitTest, MissReturnsNullopt) {
    ui::RenderScene scene;
    scene.hit_regions.push_back(
        MakeRegion(RectInt{.x = 0, .y = 0, .width = 10, .height = 10},
                   {.kind = ui::WidgetKind::UpperRow, .entity = 1, .action = std::nullopt}));

    EXPECT_FALSE(scene.HitTest(50, 50).has_value());
}

TEST(HitTest, OverlappingRegionsGiveTheOneDrawnLast) {
    ui::RenderScene scene;
    const RectInt rect{.x = 0, .y = 0, .width = 10, .height = 10};
    scene.hit_regions.push_back(
        MakeRegion(rect, {.kind = ui::WidgetKind::UpperRow, .entity = 1, .action = std::nullopt}));
    scene.hit_regions.push_back(
        MakeRegion(rect, {.kind = ui::WidgetKind::LowerRow, .entity = 2, .action = std::nullopt}));

    const auto hit = scene.HitTest(5, 5);

    if (!hit.has_value()) {
        FAIL() << "expected a hit";
        return;
    }
    EXPECT_EQ(hit.value().kind, ui::WidgetKind::LowerRow);
    EXPECT_EQ(hit.value().entity, 2U);
}

TEST(HitTest, ClearResetsHitRegions) {
    ui::RenderScene scene;
    scene.hit_regions.push_back(
        MakeRegion(RectInt{.x = 0, .y = 0, .width = 10, .height = 10},
                   {.kind = ui::WidgetKind::UpperRow, .entity = 1, .action = std::nullopt}));

    scene.Clear();

    EXPECT_FALSE(scene.HitTest(5, 5).has_value());
}

TEST(HitTest, ClearResetsOutlines) {
    ui::RenderScene scene;
    scene.outlines.push_back(
        {.rect = {.x = 0, .y = 0, .width = 10, .height = 10}, .color = {}, .thickness = 2});

    scene.Clear();

    EXPECT_TRUE(scene.outlines.empty());
}
