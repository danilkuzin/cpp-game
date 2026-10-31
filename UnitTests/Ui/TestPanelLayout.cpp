#include "PanelLayout.h"
#include "RectInt.h"
#include <cstddef>
#include <gtest/gtest.h>

namespace {
constexpr RectInt kPanel{.x = 100, .y = 100, .width = 200, .height = 200};
} // namespace

TEST(PanelLayout, RowsTileWithNoGapOrOverlap) {
    for (std::size_t row = 0; row < 5; ++row) {
        const auto rect = ui::panel_layout::RowRect(kPanel, row);
        const auto next = ui::panel_layout::RowRect(kPanel, row + 1);
        EXPECT_EQ(rect.y + rect.height, next.y);
    }
}

TEST(PanelLayout, ClickOnLastPixelOfRowResolvesToThatRow) {
    constexpr std::size_t kTargetRow = 3;
    const auto rect = ui::panel_layout::RowRect(kPanel, kTargetRow);
    const auto x = static_cast<float>(rect.x + rect.width - 1);
    const auto y = static_cast<float>(rect.y + rect.height - 1);
    const auto row = ui::panel_layout::HitTestRow(kPanel, 6, x, y);
    ASSERT_TRUE(row.has_value());
    EXPECT_EQ(row.value(), kTargetRow); // NOLINT(bugprone-unchecked-optional-access)
}

TEST(PanelLayout, ClickOutsidePanelMisses) {
    EXPECT_FALSE(ui::panel_layout::HitTestRow(kPanel, 3, 50.0F, 50.0F).has_value());
}

TEST(PanelLayout, ClickPastLastRowMisses) {
    constexpr std::size_t kRowCount = 2;
    const auto past_last_row = ui::panel_layout::RowRect(kPanel, kRowCount);
    const auto row =
        ui::panel_layout::HitTestRow(kPanel, kRowCount, static_cast<float>(past_last_row.x),
                                     static_cast<float>(past_last_row.y));
    EXPECT_FALSE(row.has_value());
}

TEST(PanelLayout, ClickInTopPaddingBeforeRowZeroMisses) {
    const auto row = ui::panel_layout::HitTestRow(kPanel, 3, static_cast<float>(kPanel.x + 10),
                                                  static_cast<float>(kPanel.y));
    EXPECT_FALSE(row.has_value());
}
