#include "Components.h"
#include "EntityFactory.h"
#include "Location.h"
#include "RegistryQueries.h"
#include "Support/Fixtures.h"
#include "Support/MakeRegistry.h"
#include <gtest/gtest.h>

TEST(EntitiesAt, FindsEntityOnCell) {
    auto registry = MakeTestRegistry();

    auto entity_id = registry.entities.CreateEntity();
    registry.Storage<Location>().Add(entity_id, {.coord = {.x = 3, .y = 4}, .map_level = 0});

    auto found = systems::EntitiesAt(registry, {.coord = {.x = 3, .y = 4}, .map_level = 0});

    ASSERT_EQ(found.size(), 1);
    EXPECT_EQ(found.front(), entity_id);
}

TEST(EntitiesAt, FindsAllEntitiesSharingACell) {
    auto registry = MakeTestRegistry();

    auto item_id = registry.entities.CreateEntity();
    registry.Storage<Location>().Add(item_id, {.coord = {.x = 3, .y = 4}, .map_level = 0});
    auto monster_id = registry.entities.CreateEntity();
    registry.Storage<component::MonsterTag>().Add(monster_id, {});
    registry.Storage<Location>().Add(monster_id, {.coord = {.x = 3, .y = 4}, .map_level = 0});

    auto found = systems::EntitiesAt(registry, {.coord = {.x = 3, .y = 4}, .map_level = 0});

    EXPECT_EQ(found.size(), 2);
}

TEST(EntitiesAt, EmptyCellReturnsEmpty) {
    auto registry = MakeTestRegistry();

    auto entity_id = registry.entities.CreateEntity();
    registry.Storage<Location>().Add(entity_id, {.coord = {.x = 3, .y = 4}, .map_level = 0});

    EXPECT_TRUE(systems::EntitiesAt(registry, {.coord = {.x = 5, .y = 5}, .map_level = 0}).empty());
}

TEST(EntitiesAt, SameCoordDifferentLevelReturnsEmpty) {
    auto registry = MakeTestRegistry();

    auto entity_id = registry.entities.CreateEntity();
    registry.Storage<Location>().Add(entity_id, {.coord = {.x = 3, .y = 4}, .map_level = 0});

    EXPECT_TRUE(systems::EntitiesAt(registry, {.coord = {.x = 3, .y = 4}, .map_level = 1}).empty());
}

TEST(BlockingEntityIdAt, ReturnsTheBodyStandingThere) {
    auto registry = MakeTestRegistry();
    const Location loc{.coord = {.x = 3, .y = 3}, .map_level = 0};
    auto monster = MakeCreatureAt(registry, loc, /*hp=*/5, /*is_monster=*/true);

    EXPECT_EQ(systems::BlockingEntityIdAt(registry, loc), monster);
    EXPECT_TRUE(systems::BlockingEntityAt(registry, loc));
}

TEST(BlockingEntityIdAt, ItemsOnTheTileDoNotBlock) {
    auto registry = MakeTestRegistry();
    const Location loc{.coord = {.x = 3, .y = 3}, .map_level = 0};
    MakeGroundItemAt(registry, loc, /*size=*/1);

    EXPECT_FALSE(systems::BlockingEntityIdAt(registry, loc).has_value());
    EXPECT_FALSE(systems::BlockingEntityAt(registry, loc));
}

TEST(TileIsEmpty, TrueForABareTile) {
    auto registry = MakeTestRegistry();

    EXPECT_TRUE(systems::TileIsEmpty(registry, {.coord = {.x = 3, .y = 3}, .map_level = 0}));
}

TEST(TileIsEmpty, FalseForATileHoldingANonBlockingEntity) {
    auto registry = MakeTestRegistry();
    const Location loc{.coord = {.x = 3, .y = 3}, .map_level = 0};
    entity::CreateTrap(registry, /*difficulty=*/10, loc);

    EXPECT_FALSE(systems::TileIsEmpty(registry, loc));
    EXPECT_FALSE(systems::BlockingEntityAt(registry, loc));
}

TEST(TileIsEmpty, GetUnoccupiedPointRefusesTilesHoldingTileFeatures) {
    auto registry = MakeTestRegistry();
    auto &map = registry.maps[0];
    for (int x = 0; x < map.Width(); x++) {
        for (int y = 0; y < map.Height(); y++) {
            entity::CreateTrap(registry, /*difficulty=*/10,
                               {.coord = {.x = x, .y = y}, .map_level = 0});
        }
    }

    EXPECT_FALSE(systems::UnoccupiedPoint(registry, /*map_level=*/0).has_value());
}
