#include "Components.h"
#include "EntityFactory.h"
#include "Event.h"
#include "Hero.h"
#include "Support/MakeRegistry.h"
#include "Systems.h"
#include "Vector2Int.h"
#include <gtest/gtest.h>

TEST(MovementSystem, MovesHeroToPassableTile) {
    auto registry = MakeTestRegistry();

    const entity::HeroGenerationParameters hparams{
        .name = "abc", .race = hero::Race::Dwarf, .hclass = hero::HClass::Mage};
    auto hero = entity::CreateHero(registry, hparams, {.coord = {.x = 5, .y = 5}, .map_level = 0});
    registry.Storage<component::IntentToMove>().Add(hero, {.dcoord = Vector2Int::Right()});

    auto result = systems::MovementSystem(registry);

    EXPECT_EQ(registry.Storage<Location>().Get(hero)->coord, (Vector2Int{.x = 6, .y = 5}));
    EXPECT_FALSE(registry.Storage<component::IntentToMove>().Has(hero));
    EXPECT_TRUE(result.events.empty());
}

TEST(MovementSystem, CannotMoveToImpassableTile) {
    auto registry = MakeTestRegistry();
    registry.maps[0].At({.x = 6, .y = 5}).terrain_id = 4;

    const entity::HeroGenerationParameters hparams{
        .name = "abc", .race = hero::Race::Dwarf, .hclass = hero::HClass::Mage};
    auto hero = entity::CreateHero(registry, hparams, {.coord = {.x = 5, .y = 5}, .map_level = 0});
    registry.Storage<component::IntentToMove>().Add(hero, {.dcoord = Vector2Int::Right()});

    auto result = systems::MovementSystem(registry);

    EXPECT_EQ(registry.Storage<Location>().Get(hero)->coord, (Vector2Int{.x = 5, .y = 5}));
    EXPECT_FALSE(registry.Storage<component::IntentToMove>().Has(hero));
    ASSERT_EQ(result.events.size(), 1);
    EXPECT_EQ(result.events[0].type, event::Type::Blocked);
    EXPECT_EQ(result.events[0].attacker, hero);
}

TEST(MovementSystem, NonPlayerBlockedMoveEmitsNoEvent) {
    auto registry = MakeTestRegistry();
    registry.maps[0].At({.x = 6, .y = 5}).terrain_id = 4;

    auto monster = registry.entities.CreateEntity();
    registry.Storage<Location>().Add(monster, {.coord = {.x = 5, .y = 5}, .map_level = 0});
    registry.Storage<component::IntentToMove>().Add(monster, {.dcoord = Vector2Int::Right()});

    auto result = systems::MovementSystem(registry);

    EXPECT_TRUE(result.events.empty());
}

TEST(MovementSystem, CannotMoveOntoOccupiedTile) {
    auto registry = MakeTestRegistry();

    const entity::HeroGenerationParameters hparams{
        .name = "abc", .race = hero::Race::Dwarf, .hclass = hero::HClass::Mage};
    auto hero = entity::CreateHero(registry, hparams, {.coord = {.x = 5, .y = 5}, .map_level = 0});

    auto blocker = registry.entities.CreateEntity();
    registry.Storage<component::MonsterTag>().Add(blocker, {});
    registry.Storage<Location>().Add(blocker, {.coord = {.x = 6, .y = 5}, .map_level = 0});

    registry.Storage<component::IntentToMove>().Add(hero, {.dcoord = Vector2Int::Right()});

    systems::MovementSystem(registry);

    EXPECT_EQ(registry.Storage<Location>().Get(hero)->coord, (Vector2Int{.x = 5, .y = 5}));
    EXPECT_EQ(registry.Storage<Location>().Get(blocker)->coord, (Vector2Int{.x = 6, .y = 5}));
    EXPECT_FALSE(registry.Storage<component::IntentToMove>().Has(hero));
}

TEST(MovementSystem, RevealsMapAroundPlayerAfterMove) {
    auto registry = MakeTestRegistry();

    const entity::HeroGenerationParameters hparams{
        .name = "abc", .race = hero::Race::Dwarf, .hclass = hero::HClass::Mage};
    auto hero = entity::CreateHero(registry, hparams, {.coord = {.x = 5, .y = 5}, .map_level = 0});
    registry.Storage<component::IntentToMove>().Add(hero, {.dcoord = Vector2Int::Right()});

    systems::MovementSystem(registry);

    const auto &map = registry.maps[0];
    EXPECT_TRUE(map.At({.x = 6 + hero::kVisionRadius, .y = 5}).explored);
    EXPECT_TRUE(map.At({.x = 6 - hero::kVisionRadius, .y = 5}).explored);
    EXPECT_TRUE(map.At({.x = 6, .y = 5 + hero::kVisionRadius}).explored);
    EXPECT_TRUE(map.At({.x = 6, .y = 5 - hero::kVisionRadius}).explored);
    EXPECT_FALSE(map.At({.x = 6 + hero::kVisionRadius + 1, .y = 5}).explored);
    EXPECT_FALSE(map.At({.x = 6, .y = 5 + hero::kVisionRadius + 1}).explored);
}

TEST(MovementSystem, NonPlayerMoveDoesNotRevealMap) {
    auto registry = MakeTestRegistry();
    registry.maps[0].At({.x = 6, .y = 5}).terrain_id = 4;

    auto entitiy_id = registry.entities.CreateEntity();
    registry.Storage<component::VisionComponent>().Add(entitiy_id,
                                                       {.view_zone = hero::kVisionRadius});
    registry.Storage<Location>().Add(entitiy_id, {.coord = {.x = 5, .y = 5}, .map_level = 0});

    systems::MovementSystem(registry);

    EXPECT_FALSE(registry.maps[0].At({.x = 6 + hero::kVisionRadius, .y = 5}).explored);
}
