#include "PanelLayout.h"
#include "PanelRects.h"
#include "RectInt.h"
#include "RenderScene.h"
#include "SidebarPanel.h"
#include "Widget.h"
#include <gtest/gtest.h>
#include <optional>
#include <vector>

namespace {
constexpr RectInt kPanelRect{.x = 0, .y = 0, .width = 100, .height = 100};
} // namespace

TEST(SidebarPanel, PushSidebarBackdropAddsAnInertPanelAndHitRegion) {
    ui::RenderScene scene;

    ui::PushSidebarBackdrop(scene);

    ASSERT_EQ(scene.panels.size(), 1U);
    EXPECT_EQ(scene.panels[0].rect.x, ui::layout::kSidebarBackdropRect.x);
    EXPECT_EQ(scene.panels[0].rect.y, ui::layout::kSidebarBackdropRect.y);
    ASSERT_EQ(scene.hit_regions.size(), 1U);
    EXPECT_EQ(scene.hit_regions[0].id.kind, ui::WidgetKind::None);
    EXPECT_FALSE(scene.hit_regions[0].id.entity.has_value());
}

TEST(SidebarPanel, PushListPanelPutsTitleFirstAndPushesNoHitRegionsWhenEmpty) {
    ui::RenderScene scene;

    ui::PushListPanel(scene, kPanelRect, "Title", {}, ui::WidgetKind::UpperRow);

    ASSERT_EQ(scene.panels.size(), 1U);
    ASSERT_FALSE(scene.panels[0].rows.empty());
    EXPECT_EQ(scene.panels[0].rows[0], "Title");
    EXPECT_TRUE(scene.hit_regions.empty());
}

TEST(SidebarPanel, PushListPanelPushesAHitRegionOnlyForRowsWithAnEntity) {
    ui::RenderScene scene;
    const std::vector<ui::ListRow> rows{{.text = "with entity", .entity = 7U},
                                        {.text = "placeholder", .entity = std::nullopt}};

    ui::PushListPanel(scene, kPanelRect, "Title", rows, ui::WidgetKind::LowerRow);

    ASSERT_EQ(scene.panels[0].rows.size(), 3U);
    EXPECT_EQ(scene.panels[0].rows[1], "with entity");
    EXPECT_EQ(scene.panels[0].rows[2], "placeholder");
    ASSERT_EQ(scene.hit_regions.size(), 1U);
    EXPECT_EQ(scene.hit_regions[0].id.kind, ui::WidgetKind::LowerRow);
    EXPECT_EQ(scene.hit_regions[0].id.entity, 7U);
}

TEST(SidebarPanel, HitRegionsLandOnTheRowAfterTheTitle) {
    ui::RenderScene scene;
    const std::vector<ui::ListRow> rows{{.text = "first", .entity = 1U},
                                        {.text = "second", .entity = 2U}};

    ui::PushListPanel(scene, kPanelRect, "Title", rows, ui::WidgetKind::UpperRow);

    const auto expected_first = ui::panel_layout::RowRect(kPanelRect, 1);
    const auto expected_second = ui::panel_layout::RowRect(kPanelRect, 2);
    ASSERT_EQ(scene.hit_regions.size(), 2U);
    EXPECT_EQ(scene.hit_regions[0].rect.y, expected_first.y);
    EXPECT_EQ(scene.hit_regions[1].rect.y, expected_second.y);
}
