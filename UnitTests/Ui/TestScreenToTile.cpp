#include "Coordinates.h"
#include "Layout.h"
#include "RectInt.h"
#include "Vector2Int.h"
#include <array>
#include <gtest/gtest.h>
#include <optional>

namespace {
void ExpectTileRoundTrips(const RectInt &view, Vector2Int tile) {
    const Vector2Int top_left = ui::TileToScreen(tile, view);
    const auto round_tripped =
        ui::ScreenToTile(static_cast<float>(top_left.x) + (ui::layout::kTileWidth / 2.F),
                         static_cast<float>(top_left.y) + (ui::layout::kTileHeight / 2.F), view);

    ASSERT_TRUE(round_tripped.has_value());
    EXPECT_EQ(round_tripped.value().x, tile.x); // NOLINT(bugprone-unchecked-optional-access)
    EXPECT_EQ(round_tripped.value().y, tile.y); // NOLINT(bugprone-unchecked-optional-access)
}
} // namespace

TEST(ScreenToTile, RoundTripsAtSeveralCameraOffsets) {
    constexpr std::array kViews = {
        RectInt{.x = 0, .y = 0, .width = 10, .height = 10},
        RectInt{.x = 7, .y = 3, .width = 10, .height = 10},
        RectInt{.x = 40, .y = 40, .width = 10, .height = 10},
    };
    constexpr std::array kOffsets = {Vector2Int{.x = 0, .y = 0}, Vector2Int{.x = 3, .y = 5},
                                     Vector2Int{.x = 9, .y = 9}};

    for (const auto &view : kViews) {
        for (const auto &offset : kOffsets) {
            ExpectTileRoundTrips(view, {.x = view.x + offset.x, .y = view.y + offset.y});
        }
    }
}

TEST(ScreenToTile, TopLeftPixelOfATileMapsToThatTile) {
    const RectInt view{.x = 0, .y = 0, .width = 10, .height = 10};

    const auto tile = ui::ScreenToTile(50.F, 50.F, view);

    ASSERT_TRUE(tile.has_value());
    EXPECT_EQ(tile.value().x, 0); // NOLINT(bugprone-unchecked-optional-access)
    EXPECT_EQ(tile.value().y, 0); // NOLINT(bugprone-unchecked-optional-access)
}

TEST(ScreenToTile, LastPixelOfATileMapsToTheSameTile) {
    const RectInt view{.x = 0, .y = 0, .width = 10, .height = 10};

    const auto tile = ui::ScreenToTile(99.F, 99.F, view);

    ASSERT_TRUE(tile.has_value());
    EXPECT_EQ(tile.value().x, 0); // NOLINT(bugprone-unchecked-optional-access)
    EXPECT_EQ(tile.value().y, 0); // NOLINT(bugprone-unchecked-optional-access)
}

TEST(ScreenToTile, WindowMarginIsNotATile) {
    const RectInt view{.x = 0, .y = 0, .width = 10, .height = 10};

    EXPECT_FALSE(ui::ScreenToTile(0.F, 60.F, view).has_value());
    EXPECT_FALSE(ui::ScreenToTile(49.F, 60.F, view).has_value());
    EXPECT_FALSE(ui::ScreenToTile(60.F, 0.F, view).has_value());
    EXPECT_FALSE(ui::ScreenToTile(60.F, 49.F, view).has_value());
}

TEST(ScreenToTile, PastTheFarEdgeOfTheViewIsNotATile) {
    const RectInt view{.x = 0, .y = 0, .width = 2, .height = 2};

    EXPECT_FALSE(ui::ScreenToTile(150.F, 50.F, view).has_value());
    EXPECT_FALSE(ui::ScreenToTile(50.F, 150.F, view).has_value());
}

TEST(ScreenToTile, NegativeCoordinatesAreNotATile) {
    const RectInt view{.x = 0, .y = 0, .width = 10, .height = 10};

    EXPECT_FALSE(ui::ScreenToTile(-1.F, 50.F, view).has_value());
    EXPECT_FALSE(ui::ScreenToTile(50.F, -1.F, view).has_value());
}

TEST(ScreenToTile, TileToScreenMatchesWhatTheSceneBuildersProduce) {
    const RectInt view{.x = 0, .y = 0, .width = 10, .height = 10};

    const auto pixel = ui::TileToScreen({.x = 0, .y = 0}, view);

    EXPECT_EQ(pixel.x, 50);
    EXPECT_EQ(pixel.y, 50);
}

TEST(Coordinates, CardinalTowardPicksTheDominantAxis) {
    const Vector2Int origin{.x = 5, .y = 7};

    const auto direction = ui::CardinalToward(origin, origin + Vector2Int{.x = 3, .y = -1});

    EXPECT_EQ(direction, Vector2Int::Right());
}

TEST(Coordinates, CardinalTowardBreaksDiagonalTiesHorizontally) {
    const Vector2Int origin{.x = 5, .y = 7};

    EXPECT_EQ(ui::CardinalToward(origin, origin + Vector2Int{.x = 2, .y = -2}),
              Vector2Int::Right());
    EXPECT_EQ(ui::CardinalToward(origin, origin + Vector2Int{.x = -2, .y = 2}), Vector2Int::Left());
}

TEST(Coordinates, CardinalTowardOfTheOriginIsNothing) {
    const Vector2Int origin{.x = 5, .y = 7};

    EXPECT_EQ(ui::CardinalToward(origin, origin), std::nullopt);
}

TEST(Coordinates, CardinalTowardIsAlwaysAUnitCardinal) {
    const Vector2Int origin{.x = 5, .y = 7};

    for (int dx = -3; dx <= 3; ++dx) {
        for (int dy = -3; dy <= 3; ++dy) {
            if (dx == 0 && dy == 0) {
                continue;
            }
            const auto direction =
                ui::CardinalToward(origin, origin + Vector2Int{.x = dx, .y = dy});

            ASSERT_TRUE(direction.has_value());
            const auto unit = direction.value(); // NOLINT(bugprone-unchecked-optional-access)
            EXPECT_TRUE(unit == Vector2Int::Up() || unit == Vector2Int::Down() ||
                        unit == Vector2Int::Left() || unit == Vector2Int::Right());
        }
    }
}
