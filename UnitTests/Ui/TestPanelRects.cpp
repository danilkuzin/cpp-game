#include "Layout.h"
#include "PanelLayout.h"
#include "PanelRects.h"
#include "RectInt.h"
#include <cstddef>
#include <gtest/gtest.h>
#include <utility>

namespace {
using ui::layout::kGameplayInfoPanelRect;
using ui::layout::kGameplayLogPanelRect;
using ui::layout::kMaxItemRows;
using ui::layout::kSidebarBackdropRect;
using ui::layout::kSidebarCloseButtonRect;
using ui::layout::kSidebarLogPanelRect;
using ui::layout::kSidebarLowerPanelRect;
using ui::layout::kSidebarUpperPanelRect;
using ui::layout::kSpellBarSlots;
using ui::layout::kSpellBarY;
using ui::layout::kSpellButtonWidth;
using ui::layout::SpellButtonRect;

std::pair<float, float> PointInRow(RectInt panel_rect, std::size_t row_index) {
    const auto rect = ui::panel_layout::RowRect(panel_rect, row_index);
    const int center_x = rect.x + (rect.width / 2);
    const int center_y = rect.y + (rect.height / 2);
    return {static_cast<float>(center_x), static_cast<float>(center_y)};
}
} // namespace

TEST(PanelRects, SidebarPanelsDoNotOverlap) {
    EXPECT_LE(kSidebarUpperPanelRect.y + kSidebarUpperPanelRect.height, kSidebarLowerPanelRect.y);
    EXPECT_LE(kSidebarLowerPanelRect.y + kSidebarLowerPanelRect.height, kSidebarLogPanelRect.y);
}

TEST(PanelRects, AFullPanelsRowsStayInsideItsRect) {
    const auto last_row = ui::panel_layout::RowRect(kSidebarUpperPanelRect, kMaxItemRows - 1);
    EXPECT_LE(last_row.y + last_row.height,
              kSidebarUpperPanelRect.y + kSidebarUpperPanelRect.height);
}

TEST(PanelRects, ClickOnLowerPanelFirstRowMissesAFullUpperPanel) {
    const auto [x, y] = PointInRow(kSidebarLowerPanelRect, 1);

    EXPECT_FALSE(
        ui::panel_layout::HitTestRow(kSidebarUpperPanelRect, kMaxItemRows, x, y).has_value());

    const auto row = ui::panel_layout::HitTestRow(kSidebarLowerPanelRect, kMaxItemRows, x, y);
    ASSERT_TRUE(row.has_value());
    EXPECT_EQ(row.value(), 1U); // NOLINT(bugprone-unchecked-optional-access)
}

TEST(PanelRects, SidebarBackdropCoversGameplayHud) {
    EXPECT_LE(kSidebarBackdropRect.y, kGameplayInfoPanelRect.y);
    EXPECT_GE(kSidebarBackdropRect.y + kSidebarBackdropRect.height,
              kGameplayLogPanelRect.y + kGameplayLogPanelRect.height);
}

TEST(PanelRects, SidebarBackdropCoversMenuPanels) {
    EXPECT_LE(kSidebarBackdropRect.y, kSidebarUpperPanelRect.y);
    EXPECT_GE(kSidebarBackdropRect.y + kSidebarBackdropRect.height,
              kSidebarLogPanelRect.y + kSidebarLogPanelRect.height);
}

TEST(PanelRects, CloseButtonIsInsideBackdropAndBelowLogPanel) {
    EXPECT_GE(kSidebarCloseButtonRect.y, kSidebarLogPanelRect.y + kSidebarLogPanelRect.height);
    EXPECT_LE(kSidebarCloseButtonRect.y + kSidebarCloseButtonRect.height,
              kSidebarBackdropRect.y + kSidebarBackdropRect.height);
}

TEST(PanelRects, SpellBarIsOnScreenAndClearOfTheSidebar) {
    EXPECT_LE(kSpellBarY + ui::layout::kButtonHeight, ui::layout::kScreenHeight);
    EXPECT_LE(SpellButtonRect(kSpellBarSlots - 1).x + kSpellButtonWidth, kSidebarBackdropRect.x);
}
