#include "Location.h"
#include "Navigation.h"
#include "Support/Fixtures.h"
#include "Support/MakeRegistry.h"
#include <gtest/gtest.h>

TEST(DistanceMap, OpenGround) {
    auto registry = MakeTestRegistry();
    const Location goal = {.coord = {.x = 5, .y = 5}, .map_level = 0};
    auto dmap = systems::BuildDistanceMap(registry, goal);

    EXPECT_EQ(dmap.At({.x = 5, .y = 5}), 0);
    EXPECT_EQ(dmap.At({.x = 5, .y = 8}), 3);
    EXPECT_EQ(dmap.At({.x = 7, .y = 7}), 4);
}

TEST(DistanceMap, WallForcesDetour) {
    auto registry = MakeTestRegistry();
    registry.maps[0].At({.x = 5, .y = 4}).terrain_id = 4;

    const Location goal = {.coord = {.x = 5, .y = 2}, .map_level = 0};
    auto dmap = systems::BuildDistanceMap(registry, goal);

    EXPECT_EQ(dmap.At({.x = 5, .y = 5}), 5);
    EXPECT_FALSE(dmap.At({.x = 5, .y = 4}).has_value());
}

TEST(DistanceMap, EnclosedUnreachable) {
    auto registry = MakeTestRegistry();
    const Location goal = {.coord = {.x = 5, .y = 5}, .map_level = 0};
    registry.maps[0].At({.x = 2, .y = 1}).terrain_id = 4;
    registry.maps[0].At({.x = 2, .y = 3}).terrain_id = 4;
    registry.maps[0].At({.x = 1, .y = 2}).terrain_id = 4;
    registry.maps[0].At({.x = 3, .y = 2}).terrain_id = 4;

    auto dmap = systems::BuildDistanceMap(registry, goal);

    EXPECT_FALSE(dmap.At({.x = 2, .y = 2}).has_value());
}

TEST(DistanceMap, CreaturesAreWalls) {
    auto registry = MakeTestRegistry();
    MakeHeroAt(registry, {.x = 5, .y = 2});
    MakeCreatureAt(registry, {.coord = {.x = 5, .y = 4}, .map_level = 0}, 1, true);
    const Location goal = {.coord = {.x = 5, .y = 2}, .map_level = 0};
    auto dmap = systems::BuildDistanceMap(registry, goal);

    EXPECT_EQ(dmap.At({.x = 5, .y = 2}), 0);
    EXPECT_FALSE(dmap.At({.x = 5, .y = 4}).has_value());
    EXPECT_EQ(dmap.At({.x = 5, .y = 5}), 5);
}
