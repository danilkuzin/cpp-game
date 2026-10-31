#pragma once

#include "Color.h"
#include "Inventory.h"
#include "Layout.h"
#include "MessageLog.h"
#include "PanelLayout.h"
#include "RectInt.h"
#include <algorithm>
#include <cstddef>

namespace ui::layout {
constexpr ui::Color kPanelBgColor{.r = 30, .g = 30, .b = 30};
constexpr ui::Color kPanelTextColor{.r = 255, .g = 0, .b = 0};

constexpr std::size_t kMaxItemRows = game_item::kMaxCarrySize + 1;    // title + carried items
constexpr std::size_t kMaxLogRows = ui::MessageLog::kMaxLogLines + 1; // title + log lines
constexpr std::size_t kGameplayInfoRows = 4;                          // hp, mana, position, money

constexpr RectInt kSidebarUpperPanelRect{
    .x = 600, .y = 100, .width = 200, .height = panel_layout::PanelHeight(kMaxItemRows)};
constexpr RectInt kSidebarLowerPanelRect{.x = 600,
                                         .y = kSidebarUpperPanelRect.y +
                                              kSidebarUpperPanelRect.height + 10,
                                         .width = 200,
                                         .height = panel_layout::PanelHeight(kMaxItemRows)};
constexpr RectInt kSidebarLogPanelRect{.x = 600,
                                       .y = kSidebarLowerPanelRect.y +
                                            kSidebarLowerPanelRect.height + 10,
                                       .width = 200,
                                       .height = panel_layout::PanelHeight(kMaxLogRows)};

static_assert(kSidebarUpperPanelRect.y + kSidebarUpperPanelRect.height <= kSidebarLowerPanelRect.y);
static_assert(kSidebarLowerPanelRect.y + kSidebarLowerPanelRect.height <= kSidebarLogPanelRect.y);
static_assert(kSidebarLogPanelRect.y + kSidebarLogPanelRect.height <= ui::layout::kScreenHeight);

constexpr RectInt kGameplayInfoPanelRect{
    .x = 600, .y = 20, .width = 200, .height = panel_layout::PanelHeight(kGameplayInfoRows)};
constexpr RectInt kGameplayLogPanelRect{
    .x = 600, .y = 300, .width = 200, .height = panel_layout::PanelHeight(kMaxLogRows)};

static_assert(kGameplayInfoPanelRect.y + kGameplayInfoPanelRect.height <= kGameplayLogPanelRect.y);
constexpr int kButtonHeight = panel_layout::PanelHeight(1);
constexpr ui::Color kSpellButtonAimedColor{.r = 120, .g = 60, .b = 20};
constexpr std::size_t kSpellBarSlots = 2;
constexpr int kSpellButtonWidth = 150;
constexpr int kSpellButtonGap = 10;
constexpr int kSpellBarY = ui::layout::kScreenHeight - kButtonHeight - 20;

constexpr RectInt SpellButtonRect(std::size_t slot) {
    return {.x = (ui::layout::kWindowLeft * ui::layout::kTileWidth) +
                 (static_cast<int>(slot) * (kSpellButtonWidth + kSpellButtonGap)),
            .y = kSpellBarY,
            .width = kSpellButtonWidth,
            .height = kButtonHeight};
}

constexpr RectInt kSidebarCloseButtonRect{.x = 600,
                                          .y = kSidebarLogPanelRect.y +
                                               kSidebarLogPanelRect.height + 10,
                                          .width = 200,
                                          .height = kButtonHeight};
constexpr int kSidebarBackdropTop = std::min(kGameplayInfoPanelRect.y, kSidebarUpperPanelRect.y);
constexpr int kSidebarBackdropBottom =
    std::max({kGameplayLogPanelRect.y + kGameplayLogPanelRect.height,
              kSidebarLogPanelRect.y + kSidebarLogPanelRect.height,
              kSidebarCloseButtonRect.y + kSidebarCloseButtonRect.height});
constexpr RectInt kSidebarBackdropRect{.x = 600,
                                       .y = kSidebarBackdropTop,
                                       .width = 200,
                                       .height = kSidebarBackdropBottom - kSidebarBackdropTop};
static_assert(kSidebarCloseButtonRect.y + kSidebarCloseButtonRect.height <=
              kSidebarBackdropRect.y + kSidebarBackdropRect.height);
static_assert(kSidebarBackdropRect.y + kSidebarBackdropRect.height <= ui::layout::kScreenHeight);

// title, hero name, level, monsters slain, money, equipment, prompt.
constexpr std::size_t kGameOverRows = 7;
constexpr RectInt kGameOverPanelRect{
    .x = 440, .y = 440, .width = 400, .height = panel_layout::PanelHeight(kGameOverRows)};
static_assert(kGameOverPanelRect.y + kGameOverPanelRect.height <= ui::layout::kScreenHeight);

static_assert(kSpellBarY + kButtonHeight <= ui::layout::kScreenHeight);
static_assert(SpellButtonRect(kSpellBarSlots - 1).x + kSpellButtonWidth <= kSidebarBackdropRect.x);

static_assert(kSidebarLogPanelRect.y + kSidebarLogPanelRect.height <= kSidebarCloseButtonRect.y);

} // namespace ui::layout
