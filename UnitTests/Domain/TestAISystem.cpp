#include "Components.h"
#include "EntityFactory.h"
#include "Hero.h"
#include "Monster.h"
#include "Support/Fixtures.h"
#include "Support/MakeRegistry.h"
#include "Systems.h"
#include "Vector2Int.h"
#include <gtest/gtest.h>

TEST(AISystem, MonsterInViewStepsTowardHero) {
    auto registry = MakeTestRegistry();

    const entity::HeroGenerationParameters hparams{
        .name = "abc", .race = hero::Race::Dwarf, .hclass = hero::HClass::Mage};
    entity::CreateHero(registry, hparams, {.coord = {.x = 5, .y = 5}, .map_level = 0});

    const monster::MonsterPrefab prefab{.id = "rat",
                                        .name = "Rat",
                                        .map_level = 0,
                                        .view_zone = 5,
                                        .max_hp = 1,
                                        .chars = {1, 1, 0, 0, 0},
                                        .tile = "../rat.bmp"};
    auto monster =
        entity::CreateMonster(registry, prefab, {.coord = {.x = 5, .y = 8}, .map_level = 0});

    systems::AISystem(registry);
    systems::MovementSystem(registry);

    EXPECT_EQ(registry.Storage<Location>().Get(monster)->coord, (Vector2Int{.x = 5, .y = 7}));
}

TEST(AISystem, MonsterOutOfViewDoesNotMove) {
    auto registry = MakeTestRegistry();

    const entity::HeroGenerationParameters hparams{
        .name = "abc", .race = hero::Race::Dwarf, .hclass = hero::HClass::Mage};
    entity::CreateHero(registry, hparams, {.coord = {.x = 5, .y = 5}, .map_level = 0});

    const monster::MonsterPrefab prefab{.id = "rat",
                                        .name = "Rat",
                                        .map_level = 0,
                                        .view_zone = 1,
                                        .max_hp = 1,
                                        .chars = {1, 1, 0, 0, 0},
                                        .tile = "../rat.bmp"};
    auto monster =
        entity::CreateMonster(registry, prefab, {.coord = {.x = 5, .y = 8}, .map_level = 0});

    systems::AISystem(registry);
    systems::MovementSystem(registry);

    EXPECT_EQ(registry.Storage<Location>().Get(monster)->coord, (Vector2Int{.x = 5, .y = 8}));
}

TEST(AISystem, ShopkeeperInViewDoesNotMove) {
    auto registry = MakeTestRegistry();

    const entity::HeroGenerationParameters hparams{
        .name = "abc", .race = hero::Race::Dwarf, .hclass = hero::HClass::Mage};
    entity::CreateHero(registry, hparams, {.coord = {.x = 5, .y = 5}, .map_level = 0});

    const monster::MonsterPrefab prefab{.id = "shopkeeper",
                                        .name = "Shopkeeper",
                                        .map_level = 0,
                                        .view_zone = 5,
                                        .max_hp = 35,
                                        .chars = {0, 0, 0, 0, 5},
                                        .tile = "shopkeeper.bmp",
                                        .shopkeeper = true};
    auto shopkeeper =
        entity::CreateMonster(registry, prefab, {.coord = {.x = 5, .y = 8}, .map_level = 0});

    systems::AISystem(registry);
    systems::MovementSystem(registry);

    EXPECT_EQ(registry.Storage<Location>().Get(shopkeeper)->coord, (Vector2Int{.x = 5, .y = 8}));
}

TEST(AISystem, ViewZoneCountsCardinalSteps) {
    auto registry = MakeTestRegistry();
    MakeHeroAt(registry, {.x = 5, .y = 5});
    auto three_steps = MakeCreatureAt(registry, {.coord = {.x = 7, .y = 6}, .map_level = 0},
                                      /*hp=*/1, /*is_monster=*/true);
    auto four_steps = MakeCreatureAt(registry, {.coord = {.x = 7, .y = 7}, .map_level = 0},
                                     /*hp=*/1, /*is_monster=*/true);
    registry.Storage<component::VisionComponent>().Add(three_steps, {.view_zone = 3});
    registry.Storage<component::VisionComponent>().Add(four_steps, {.view_zone = 3});

    systems::AISystem(registry);

    EXPECT_TRUE(registry.Storage<component::IntentToMove>().Has(three_steps));
    EXPECT_FALSE(registry.Storage<component::IntentToMove>().Has(four_steps));
}
