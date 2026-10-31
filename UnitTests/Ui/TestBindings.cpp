#include "Bindings.h"
#include "InputEvent.h"
#include <gtest/gtest.h>
#include <optional>

TEST(Bindings, UpIsContextDependent) {
    EXPECT_EQ(ui::Resolve(ui::InputContext::Gameplay, ui::Key::Up), ui::Action::MoveNorth);
    EXPECT_EQ(ui::Resolve(ui::InputContext::Menu, ui::Key::Up), ui::Action::PrevOption);
}

TEST(Bindings, EscapeCancelsInEveryContext) {
    EXPECT_EQ(ui::Resolve(ui::InputContext::Gameplay, ui::Key::Escape), ui::Action::Cancel);
    EXPECT_EQ(ui::Resolve(ui::InputContext::Menu, ui::Key::Escape), ui::Action::Cancel);
    EXPECT_EQ(ui::Resolve(ui::InputContext::TextEntry, ui::Key::Escape), ui::Action::Cancel);
}

TEST(Bindings, ReturnAndSpaceConfirmInMenu) {
    EXPECT_EQ(ui::Resolve(ui::InputContext::Menu, ui::Key::Return), ui::Action::Confirm);
    EXPECT_EQ(ui::Resolve(ui::InputContext::Menu, ui::Key::Space), ui::Action::Confirm);
}

TEST(Bindings, BackspaceOnlyResolvesInTextEntry) {
    EXPECT_EQ(ui::Resolve(ui::InputContext::TextEntry, ui::Key::Backspace), ui::Action::DeleteChar);
    EXPECT_EQ(ui::Resolve(ui::InputContext::Gameplay, ui::Key::Backspace), std::nullopt);
}

TEST(Bindings, UnknownKeyResolvesToNothingInEveryContext) {
    EXPECT_EQ(ui::Resolve(ui::InputContext::Gameplay, ui::Key::Unknown), std::nullopt);
    EXPECT_EQ(ui::Resolve(ui::InputContext::Menu, ui::Key::Unknown), std::nullopt);
    EXPECT_EQ(ui::Resolve(ui::InputContext::TextEntry, ui::Key::Unknown), std::nullopt);
}

TEST(Bindings, ReturnConfirmsInGameplay) {
    EXPECT_EQ(ui::Resolve(ui::InputContext::Gameplay, ui::Key::Return), ui::Action::Confirm);
}
