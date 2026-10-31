#include "RectInt.h"
#include <gtest/gtest.h>

TEST(RectInt, ContainsPointInsideTheRect) {
    const RectInt rect{.x = 0, .y = 0, .width = 10, .height = 10};
    EXPECT_TRUE(rect.ContainsPoint({.x = 5, .y = 5}));
}

TEST(RectInt, TopLeftCornerIsInsideBottomRightCornerIsNot) {
    const RectInt rect{.x = 0, .y = 0, .width = 10, .height = 10};
    EXPECT_TRUE(rect.ContainsPoint({.x = 0, .y = 0}));
    EXPECT_FALSE(rect.ContainsPoint({.x = 10, .y = 10}));
}

TEST(RectInt, LastContainedColumnAndRowAreInside) {
    const RectInt rect{.x = 0, .y = 0, .width = 10, .height = 10};
    EXPECT_TRUE(rect.ContainsPoint({.x = 9, .y = 9}));
}

TEST(RectInt, PointOutsideOnEachSideIsRejected) {
    const RectInt rect{.x = 0, .y = 0, .width = 10, .height = 10};
    EXPECT_FALSE(rect.ContainsPoint({.x = -1, .y = 5}));
    EXPECT_FALSE(rect.ContainsPoint({.x = 5, .y = -1}));
    EXPECT_FALSE(rect.ContainsPoint({.x = 10, .y = 5}));
    EXPECT_FALSE(rect.ContainsPoint({.x = 5, .y = 10}));
}

TEST(RectInt, OffsetOriginShiftsTheContainedRange) {
    const RectInt rect{.x = 5, .y = 5, .width = 4, .height = 4};
    EXPECT_TRUE(rect.ContainsPoint({.x = 5, .y = 5}));
    EXPECT_TRUE(rect.ContainsPoint({.x = 8, .y = 8}));
    EXPECT_FALSE(rect.ContainsPoint({.x = 9, .y = 9}));
    EXPECT_FALSE(rect.ContainsPoint({.x = 4, .y = 4}));
}
