#include "Vector2Int.h"
#include <gtest/gtest.h>

TEST(Vector2Int, DirectionHelpersMoveOneStepOnASingleAxis) {
    EXPECT_EQ(Vector2Int::Left(), (Vector2Int{.x = -1, .y = 0}));
    EXPECT_EQ(Vector2Int::Right(), (Vector2Int{.x = 1, .y = 0}));
    EXPECT_EQ(Vector2Int::Up(), (Vector2Int{.x = 0, .y = -1}));
    EXPECT_EQ(Vector2Int::Down(), (Vector2Int{.x = 0, .y = 1}));
}

TEST(Vector2Int, DistanceToIsManhattanNotEuclideanOrChebyshev) {
    const Vector2Int origin{.x = 0, .y = 0};
    EXPECT_EQ(origin.DistanceTo(Vector2Int{.x = 3, .y = 4}), 7);
}
