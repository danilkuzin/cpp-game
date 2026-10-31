#include "Coordinates.h"
#include "Layout.h"
#include "RectInt.h"
#include "Vector2Int.h"
#include <array>
#include <gtest/gtest.h>

namespace {
constexpr int kThickness = 2;

void ExpectEdgesWithin(const std::array<RectInt, 4> &edges, RectInt rect) {
    for (const RectInt &edge : edges) {
        EXPECT_GE(edge.x, rect.x);
        EXPECT_GE(edge.y, rect.y);
        EXPECT_LE(edge.x + edge.width, rect.x + rect.width);
        EXPECT_LE(edge.y + edge.height, rect.y + rect.height);
    }
}
} // namespace

TEST(OutlineEdges, FollowsTheRectItIsGivenNotTheTileSize) {
    constexpr RectInt kRect{.x = 10, .y = 20, .width = 200, .height = 30};
    const auto edges = ui::OutlineEdges(kRect, kThickness);

    const RectInt top = edges[0];
    const RectInt right = edges[1];
    const RectInt bottom = edges[2];
    const RectInt left = edges[3];

    EXPECT_EQ(top.width, kRect.width);
    EXPECT_EQ(bottom.width, kRect.width);
    EXPECT_EQ(left.height, kRect.height);
    EXPECT_EQ(right.height, kRect.height);

    EXPECT_EQ(right.x, kRect.x + kRect.width - kThickness);
    EXPECT_EQ(bottom.y, kRect.y + kRect.height - kThickness);

    ExpectEdgesWithin(edges, kRect);
}

TEST(OutlineEdges, EachEdgeIsThicknessThickOnItsShortAxis) {
    constexpr RectInt kRect{.x = 0, .y = 0, .width = 64, .height = 48};
    const auto edges = ui::OutlineEdges(kRect, kThickness);

    EXPECT_EQ(edges[0].height, kThickness);
    EXPECT_EQ(edges[1].width, kThickness);
    EXPECT_EQ(edges[2].height, kThickness);
    EXPECT_EQ(edges[3].width, kThickness);
}

TEST(OutlineEdges, EdgesSpanTheFullTileOnEachSide) {
    constexpr RectInt kView{.x = 0, .y = 0, .width = 8, .height = 8};
    const RectInt tile = ui::TileRect(Vector2Int{.x = 3, .y = 4}, kView);
    const auto edges = ui::OutlineEdges(tile, kThickness);

    EXPECT_EQ(edges[0].width, ui::layout::kTileWidth);
    EXPECT_EQ(edges[1].x, tile.x + ui::layout::kTileWidth - kThickness);
    EXPECT_EQ(edges[2].y, tile.y + ui::layout::kTileHeight - kThickness);
    EXPECT_EQ(edges[3].height, ui::layout::kTileHeight);

    ExpectEdgesWithin(edges, tile);
}
