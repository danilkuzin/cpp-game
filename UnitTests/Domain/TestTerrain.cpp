#include "Terrain.h"
#include <gtest/gtest.h>

TEST(Terrain, DefaultRegistryMapsEachIdToItsSprite) {
    const map::TerrainRegistry registry;

    ASSERT_EQ(registry.terrains.size(), 6U);
    EXPECT_EQ(registry.Get(0).sprite, "tile_grass.bmp");
    EXPECT_EQ(registry.Get(1).sprite, "tile_ground.bmp");
    EXPECT_EQ(registry.Get(2).sprite, "tile_trap.bmp");
    EXPECT_EQ(registry.Get(3).sprite, "tile_live.bmp");
    EXPECT_EQ(registry.Get(4).sprite, "tile_tree.bmp");
    EXPECT_EQ(registry.Get(5).sprite, "tile_stone.bmp");
}

TEST(Terrain, OnlyTreeAndStoneAreImpassable) {
    const map::TerrainRegistry registry;

    EXPECT_TRUE(registry.Get(0).passable);
    EXPECT_TRUE(registry.Get(1).passable);
    EXPECT_TRUE(registry.Get(2).passable);
    EXPECT_TRUE(registry.Get(3).passable);
    EXPECT_FALSE(registry.Get(4).passable);
    EXPECT_FALSE(registry.Get(5).passable);
}
