#include "Camera.h"
#include <gtest/gtest.h>

using ui::Camera;

TEST(Camera, ViewReturnsTheRectPassedToTheConstructor) {
    const Camera camera({.x = 1, .y = 2, .width = 10, .height = 8}, /*scroll_delta=*/1);
    EXPECT_EQ(camera.View().x, 1);
    EXPECT_EQ(camera.View().y, 2);
    EXPECT_EQ(camera.View().width, 10);
    EXPECT_EQ(camera.View().height, 8);
}

TEST(Camera, CenterAtPlacesTheCoordInTheMiddleOfTheView) {
    Camera camera({.x = 0, .y = 0, .width = 10, .height = 10}, /*scroll_delta=*/1);
    camera.CenterAt({.x = 20, .y = 30});
    EXPECT_EQ(camera.View().x, 15);
    EXPECT_EQ(camera.View().y, 25);
}

TEST(Camera, IsCloseToEdgeIsSymmetricAboutBothEdges) {
    const Camera camera({.x = 0, .y = 0, .width = 10, .height = 10}, /*scroll_delta=*/2);

    EXPECT_TRUE(camera.IsCloseToEdge({.x = 0, .y = 5}));
    EXPECT_TRUE(camera.IsCloseToEdge({.x = 1, .y = 5}));
    for (int x = 2; x <= 7; ++x) {
        EXPECT_FALSE(camera.IsCloseToEdge({.x = x, .y = 5})) << "x=" << x;
    }
    EXPECT_TRUE(camera.IsCloseToEdge({.x = 8, .y = 5}));
    EXPECT_TRUE(camera.IsCloseToEdge({.x = 9, .y = 5}));
}

TEST(Camera, IsCloseToEdgeIsFalseInTheMiddleOfALargeView) {
    const Camera camera({.x = 0, .y = 0, .width = 20, .height = 20}, /*scroll_delta=*/2);
    EXPECT_FALSE(camera.IsCloseToEdge({.x = 10, .y = 10}));
}

TEST(Camera, CenterAtLeavesTheHeroOutsideTheScrollZone) {
    Camera camera({.x = 0, .y = 0, .width = 10, .height = 10}, /*scroll_delta=*/2);
    camera.CenterAt({.x = 20, .y = 30});
    EXPECT_FALSE(camera.IsCloseToEdge({.x = 20, .y = 30}));
}
