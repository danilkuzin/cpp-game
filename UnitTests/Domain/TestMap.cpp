#include "Map.h"
#include "RectInt.h"
#include "Support/MakeRegistry.h"
#include <gtest/gtest.h>
#include <stdexcept>

TEST(Map, ContainsAcceptsEveryCellAndRejectsEveryNeighbourOfTheEdge) {
    auto registry = MakeTestRegistry();
    const auto &map = registry.maps[0];
    ASSERT_EQ(map.Width(), 10);
    ASSERT_EQ(map.Height(), 10);

    EXPECT_TRUE(map.Contains({.x = 0, .y = 0}));
    EXPECT_TRUE(map.Contains({.x = 9, .y = 9}));
    EXPECT_TRUE(map.Contains({.x = 0, .y = 9}));
    EXPECT_TRUE(map.Contains({.x = 9, .y = 0}));

    EXPECT_FALSE(map.Contains({.x = -1, .y = 5}));
    EXPECT_FALSE(map.Contains({.x = 5, .y = -1}));
    EXPECT_FALSE(map.Contains({.x = 10, .y = 5}));
    EXPECT_FALSE(map.Contains({.x = 5, .y = 10}));
}

TEST(Map, AtAddressesEachCellIndependently) {
    map::Map map(4, 3, map::Cell{.terrain_id = 0, .explored = false});

    map.At({.x = 3, .y = 1}).terrain_id = 5;

    EXPECT_EQ(map.At({.x = 3, .y = 1}).terrain_id, 5);
    int touched = 0;
    for (int x = 0; x < map.Width(); x++) {
        for (int y = 0; y < map.Height(); y++) {
            if (map.At({.x = x, .y = y}).terrain_id != 0) {
                touched++;
            }
        }
    }
    EXPECT_EQ(touched, 1);
}

TEST(Map, AtThrowsOutsideTheMap) {
    map::Map map(4, 3, map::Cell{.terrain_id = 0, .explored = false});

    EXPECT_THROW((void)map.At({.x = 4, .y = 0}), std::out_of_range);
    EXPECT_THROW((void)map.At({.x = 0, .y = 3}), std::out_of_range);
    EXPECT_THROW((void)map.At({.x = -1, .y = 0}), std::out_of_range);
    EXPECT_THROW((void)map.At({.x = 3, .y = 3}), std::out_of_range);
}

TEST(Map, BoundsIsTheWholeGridAtTheOrigin) {
    const map::Map map(4, 3, map::Cell{.terrain_id = 0, .explored = false});

    const RectInt bounds = map.Bounds();
    EXPECT_EQ(bounds.x, 0);
    EXPECT_EQ(bounds.y, 0);
    EXPECT_EQ(bounds.width, 4);
    EXPECT_EQ(bounds.height, 3);
}

TEST(Map, ContainsRejectsEverythingOnAnEmptyMap) {
    const map::Map map{};

    EXPECT_EQ(map.Width(), 0);
    EXPECT_EQ(map.Height(), 0);
    EXPECT_FALSE(map.Contains({.x = 0, .y = 0}));
}

TEST(Map, SetAreaVisibleClipsToTheMap) {
    auto registry = MakeTestRegistry();
    auto &map = registry.maps[0];

    map.SetAreaVisible({.x = -2, .y = -2, .width = 4, .height = 4});

    EXPECT_TRUE(map.At({.x = 0, .y = 0}).explored);
    EXPECT_TRUE(map.At({.x = 1, .y = 1}).explored);
    EXPECT_FALSE(map.At({.x = 2, .y = 2}).explored);
}
