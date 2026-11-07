#include "Components.h"
#include "EntityFactory.h"
#include "Event.h"
#include "Hero.h"
#include "Location.h"
#include "Registry.h"
#include "RegistryQueries.h"
#include "Support/Fixtures.h"
#include "Support/MakeRegistry.h"
#include "Systems.h"
#include "Vector2Int.h"
#include <gtest/gtest.h>

TEST(CollisionSystem, AreEnemiesAllOptions) {
    domain::Registry registry;

    auto hero1_id = registry.entities.CreateEntity();
    registry.Storage<component::PlayerTag>().Add(hero1_id, {});
    auto monster1_id = registry.entities.CreateEntity();
    registry.Storage<component::MonsterTag>().Add(monster1_id, {});
    auto monster2_id = registry.entities.CreateEntity();
    registry.Storage<component::MonsterTag>().Add(monster2_id, {});

    EXPECT_TRUE(systems::AreEnemies(registry, hero1_id, monster1_id));
    EXPECT_TRUE(systems::AreEnemies(registry, monster2_id, hero1_id));
    EXPECT_FALSE(systems::AreEnemies(registry, monster1_id, monster2_id));
}

TEST(CollisionSystem, CreateMeleeIntentForEnemy) {
    auto registry = MakeTestRegistry();

    const entity::HeroGenerationParameters hparams{
        .name = "abc", .race = hero::Race::Dwarf, .hclass = hero::HClass::Mage};
    auto hero_id =
        entity::CreateHero(registry, hparams, {.coord = {.x = 5, .y = 5}, .map_level = 0});

    auto monster_id = registry.entities.CreateEntity();
    registry.Storage<component::MonsterTag>().Add(monster_id, {});
    registry.Storage<Location>().Add(monster_id, {.coord = {.x = 6, .y = 5}, .map_level = 0});

    registry.Storage<component::IntentToMove>().Add(hero_id, {.dcoord = Vector2Int::Right()});

    systems::CollisionSystem(registry);

    EXPECT_FALSE(registry.Storage<component::IntentToMove>().Has(hero_id));
    ASSERT_TRUE(registry.Storage<component::IntentToMelee>().Has(hero_id));
    EXPECT_EQ(registry.Storage<component::IntentToMelee>().Get(hero_id)->target_entity_id,
              monster_id);
}

TEST(CollisionSystem, MonsterCreatesMeleeIntentForHero) {
    auto registry = MakeTestRegistry();

    const entity::HeroGenerationParameters hparams{
        .name = "abc", .race = hero::Race::Dwarf, .hclass = hero::HClass::Mage};
    auto hero_id =
        entity::CreateHero(registry, hparams, {.coord = {.x = 6, .y = 5}, .map_level = 0});

    auto monster_id = registry.entities.CreateEntity();
    registry.Storage<component::MonsterTag>().Add(monster_id, {});
    registry.Storage<Location>().Add(monster_id, {.coord = {.x = 5, .y = 5}, .map_level = 0});

    registry.Storage<component::IntentToMove>().Add(monster_id, {.dcoord = Vector2Int::Right()});

    systems::CollisionSystem(registry);

    EXPECT_FALSE(registry.Storage<component::IntentToMove>().Has(monster_id));
    ASSERT_TRUE(registry.Storage<component::IntentToMelee>().Has(monster_id));
    EXPECT_EQ(registry.Storage<component::IntentToMelee>().Get(monster_id)->target_entity_id,
              hero_id);
}

TEST(CollisionSystem, DoNotCreateMeleeIntentForEmpty) {
    auto registry = MakeTestRegistry();

    const entity::HeroGenerationParameters hparams{
        .name = "abc", .race = hero::Race::Dwarf, .hclass = hero::HClass::Mage};
    auto hero_id =
        entity::CreateHero(registry, hparams, {.coord = {.x = 5, .y = 5}, .map_level = 0});

    registry.Storage<component::IntentToMove>().Add(hero_id, {.dcoord = Vector2Int::Right()});

    systems::CollisionSystem(registry);

    EXPECT_TRUE(registry.Storage<component::IntentToMove>().Has(hero_id));
    EXPECT_FALSE(registry.Storage<component::IntentToMelee>().Has(hero_id));
}

TEST(CollisionSystem, TileWithItemAndMonsterTargetsTheMonster) {
    auto registry = MakeTestRegistry();

    const entity::HeroGenerationParameters hparams{
        .name = "abc", .race = hero::Race::Dwarf, .hclass = hero::HClass::Mage};
    auto hero_id =
        entity::CreateHero(registry, hparams, {.coord = {.x = 5, .y = 5}, .map_level = 0});

    auto item_id = registry.entities.CreateEntity();
    registry.Storage<component::ItemTag>().Add(item_id, {});
    registry.Storage<Location>().Add(item_id, {.coord = {.x = 6, .y = 5}, .map_level = 0});

    auto monster_id = registry.entities.CreateEntity();
    registry.Storage<component::MonsterTag>().Add(monster_id, {});
    registry.Storage<Location>().Add(monster_id, {.coord = {.x = 6, .y = 5}, .map_level = 0});

    registry.Storage<component::IntentToMove>().Add(hero_id, {.dcoord = Vector2Int::Right()});

    systems::CollisionSystem(registry);

    EXPECT_FALSE(registry.Storage<component::IntentToMove>().Has(hero_id));
    ASSERT_TRUE(registry.Storage<component::IntentToMelee>().Has(hero_id));
    EXPECT_EQ(registry.Storage<component::IntentToMelee>().Get(hero_id)->target_entity_id,
              monster_id);
}

TEST(CollisionSystem, HeroBumpingShopkeeperOpensTradeInsteadOfMelee) {
    auto registry = MakeTestRegistry();

    const entity::HeroGenerationParameters hparams{
        .name = "abc", .race = hero::Race::Dwarf, .hclass = hero::HClass::Mage};
    auto hero_id =
        entity::CreateHero(registry, hparams, {.coord = {.x = 5, .y = 5}, .map_level = 0});

    auto shopkeeper_id = registry.entities.CreateEntity();
    registry.Storage<component::MonsterTag>().Add(shopkeeper_id, {});
    registry.Storage<component::ShopkeeperTag>().Add(shopkeeper_id, {});
    registry.Storage<Location>().Add(shopkeeper_id, {.coord = {.x = 6, .y = 5}, .map_level = 0});

    registry.Storage<component::IntentToMove>().Add(hero_id, {.dcoord = Vector2Int::Right()});

    auto result = systems::CollisionSystem(registry);

    EXPECT_FALSE(registry.Storage<component::IntentToMove>().Has(hero_id));
    EXPECT_FALSE(registry.Storage<component::IntentToMelee>().Has(hero_id));
    ASSERT_EQ(result.events.size(), 1);
    EXPECT_EQ(result.events[0].type, event::Type::Trade);
    EXPECT_EQ(result.events[0].attacker, hero_id);
    EXPECT_EQ(result.events[0].defender, shopkeeper_id);
}

TEST(CollisionSystem, MonsterBumpingShopkeeperDoesNotOpenTrade) {
    auto registry = MakeTestRegistry();

    auto shopkeeper_id = registry.entities.CreateEntity();
    registry.Storage<component::MonsterTag>().Add(shopkeeper_id, {});
    registry.Storage<component::ShopkeeperTag>().Add(shopkeeper_id, {});
    registry.Storage<Location>().Add(shopkeeper_id, {.coord = {.x = 6, .y = 5}, .map_level = 0});

    auto monster_id = registry.entities.CreateEntity();
    registry.Storage<component::MonsterTag>().Add(monster_id, {});
    registry.Storage<Location>().Add(monster_id, {.coord = {.x = 5, .y = 5}, .map_level = 0});

    registry.Storage<component::IntentToMove>().Add(monster_id, {.dcoord = Vector2Int::Right()});

    auto result = systems::CollisionSystem(registry);

    EXPECT_TRUE(registry.Storage<component::IntentToMove>().Has(monster_id));
    EXPECT_FALSE(registry.Storage<component::IntentToMelee>().Has(monster_id));
    EXPECT_TRUE(result.events.empty());
}

TEST(CollisionSystem, DoNotCreateMeleeIntentForFriend) {
    auto registry = MakeTestRegistry();

    auto monster1_id = registry.entities.CreateEntity();
    registry.Storage<component::MonsterTag>().Add(monster1_id, {});
    registry.Storage<Location>().Add(monster1_id, {.coord = {.x = 6, .y = 5}, .map_level = 0});
    auto monster2_id = registry.entities.CreateEntity();
    registry.Storage<component::MonsterTag>().Add(monster2_id, {});
    registry.Storage<Location>().Add(monster2_id, {.coord = {.x = 7, .y = 5}, .map_level = 0});

    registry.Storage<component::IntentToMove>().Add(monster1_id, {.dcoord = Vector2Int::Right()});

    systems::CollisionSystem(registry);

    EXPECT_TRUE(registry.Storage<component::IntentToMove>().Has(monster1_id));
    EXPECT_FALSE(registry.Storage<component::IntentToMelee>().Has(monster1_id));
}

TEST(OpportunityAttackSystem, AdjacentGetsFreeHit) {
    auto registry = MakeTestRegistry();
    auto hero = MakeHeroAt(registry, {.x = 5, .y = 5});
    registry.Storage<component::IntentToMove>().Add(hero, {.dcoord = Vector2Int::Right()});
    auto id = MakeCreatureAt(registry, {.coord = {.x = 4, .y = 5}, .map_level = 0}, 1, true);
    registry.Storage<component::VisionComponent>().Add(id, {.view_zone = 5});

    systems::OpportunityAttackSystem(registry);

    const auto *melee = registry.Storage<component::IntentToMelee>().Get(id);
    ASSERT_NE(melee, nullptr);
    EXPECT_EQ(melee->target_entity_id, hero);
}
