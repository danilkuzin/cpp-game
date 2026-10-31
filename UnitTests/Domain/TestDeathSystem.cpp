#include "Components.h"
#include "EffectSystems.h"
#include "EntityStorage.h"
#include "Event.h"
#include "GameItem.h"
#include "Hero.h"
#include "Leveling.h"
#include "Location.h"
#include "Registry.h"
#include "Support/Fixtures.h"
#include "Support/TestData.h"
#include "Vector2Int.h"
#include <gtest/gtest.h>
#include <optional>

namespace {
entity::EntityId MakeCreature(domain::Registry &registry, int hp, bool is_monster) {
    return MakeCreatureAt(registry, {.coord = {.x = 1, .y = 1}, .map_level = 0}, hp, is_monster);
}
} // namespace

TEST(DeathSystem, RemovesDeadMonsterFromAllStorages) {
    domain::Registry registry;
    const auto db = MakeTestDatabase();
    auto id = MakeCreature(registry, 0, true);

    systems::DeathSystem(registry, db);

    EXPECT_FALSE(registry.Storage<component::HealthComponent>().Has(id));
    EXPECT_FALSE(registry.Storage<component::AttributesComponent>().Has(id));
    EXPECT_FALSE(registry.Storage<Location>().Has(id));
    EXPECT_FALSE(registry.Storage<component::MonsterTag>().Has(id));
}

TEST(DeathSystem, DeadMonsterEmitsMonsterSlainedEventWithDefender) {
    domain::Registry registry;
    const auto db = MakeTestDatabase();
    auto id = MakeCreature(registry, 0, true);

    auto result = systems::DeathSystem(registry, db);

    ASSERT_EQ(result.events.size(), 1U);
    const auto &event = result.events[0];
    EXPECT_EQ(event.type, event::Type::MonsterSlained);
    ASSERT_TRUE(event.defender.has_value());
    if (event.defender.has_value()) {
        EXPECT_EQ(event.defender.value(), id);
    }
}

TEST(DeathSystem, DeadHeroEmitsHeroDied) {
    domain::Registry registry;
    const auto db = MakeTestDatabase();
    auto id = MakeCreature(registry, -3, false);

    auto result = systems::DeathSystem(registry, db);

    ASSERT_EQ(result.events.size(), 1U);
    EXPECT_EQ(result.events[0].type, event::Type::HeroDied);
    EXPECT_FALSE(registry.Storage<component::HealthComponent>().Has(id));
    EXPECT_FALSE(registry.Storage<Location>().Has(id));
    EXPECT_FALSE(registry.Storage<component::PlayerTag>().Has(id));
}

TEST(DeathSystem, LivingEntityUntouched) {
    domain::Registry registry;
    const auto db = MakeTestDatabase();
    auto id = MakeCreature(registry, 5, true);

    auto result = systems::DeathSystem(registry, db);

    EXPECT_TRUE(registry.Storage<component::HealthComponent>().Has(id));
    EXPECT_TRUE(result.events.empty());
}

TEST(DeathSystem, DropsOwnedItemsAtCreaturesLocationOnDeath) {
    domain::Registry registry;
    const auto db = MakeTestDatabase();
    auto id = MakeCreature(registry, 0, true);
    auto sword = registry.entities.CreateEntity();
    registry.Storage<game_item::OwnershipComponent>().Add(sword, {.owner = id});
    registry.Storage<game_item::SizeComponent>().Add(sword, {.size = 2});
    registry.Storage<game_item::EquippedComponent>().Add(sword,
                                                         {.slot = game_item::EquipSlot::MainHand});

    systems::DeathSystem(registry, db);

    EXPECT_FALSE(registry.Storage<game_item::OwnershipComponent>().Has(sword));
    EXPECT_FALSE(registry.Storage<game_item::EquippedComponent>().Has(sword));
    ASSERT_TRUE(registry.Storage<Location>().Has(sword));
    EXPECT_EQ(registry.Storage<Location>().Get(sword)->coord, (Vector2Int{.x = 1, .y = 1}));
}

TEST(DeathSystem, HeroKillCreditsXpAndTagsMonsterSlainedWithAttacker) {
    domain::Registry registry;
    const auto db = MakeTestDatabase();
    auto hero_id = registry.entities.CreateEntity();
    registry.Storage<hero::HeroExperience>().Add(
        hero_id, {.xp = 0, .level = 1, .hclass = hero::HClass::Warrior});
    auto monster = MakeCreature(registry, 0, true);
    registry.Storage<component::IntentToTakeDamage>().Add(monster,
                                                          {.amount = 3, .attacker = hero_id});

    auto result = systems::DeathSystem(registry, db);

    EXPECT_EQ(registry.Storage<hero::HeroExperience>().Get(hero_id)->xp, leveling::kKillXp);
    ASSERT_EQ(result.events.size(), 1U);
    const auto &event = result.events[0];
    ASSERT_TRUE(event.attacker.has_value());
    if (event.attacker.has_value()) {
        EXPECT_EQ(event.attacker.value(), hero_id);
    }
}

TEST(DeathSystem, TrapKillWithNoAttackerAwardsNoXp) {
    domain::Registry registry;
    const auto db = MakeTestDatabase();
    auto hero_id = registry.entities.CreateEntity();
    registry.Storage<hero::HeroExperience>().Add(
        hero_id, {.xp = 0, .level = 1, .hclass = hero::HClass::Warrior});
    auto monster = MakeCreature(registry, 0, true);
    registry.Storage<component::IntentToTakeDamage>().Add(monster, {.amount = 3});

    auto result = systems::DeathSystem(registry, db);

    EXPECT_EQ(registry.Storage<hero::HeroExperience>().Get(hero_id)->xp, 0);
    ASSERT_EQ(result.events.size(), 1U);
    EXPECT_FALSE(result.events[0].attacker.has_value());
}

TEST(DeathSystem, AttackerWithoutHeroExperienceAwardsNoXpAndDoesNotCrash) {
    domain::Registry registry;
    const auto db = MakeTestDatabase();
    auto attacker = registry.entities.CreateEntity();
    auto monster = MakeCreature(registry, 0, true);
    registry.Storage<component::IntentToTakeDamage>().Add(monster,
                                                          {.amount = 3, .attacker = attacker});

    auto result = systems::DeathSystem(registry, db);

    ASSERT_EQ(result.events.size(), 1U);
    const auto &event = result.events[0];
    ASSERT_TRUE(event.attacker.has_value());
    if (event.attacker.has_value()) {
        EXPECT_EQ(event.attacker.value(), attacker);
    }
}

TEST(DeathSystem, MultipleDeadRemovedWithoutInvalidation) {
    domain::Registry registry;
    const auto db = MakeTestDatabase();
    auto a = MakeCreature(registry, 0, true);
    auto b = MakeCreature(registry, 0, true);
    auto c = MakeCreature(registry, 0, false);

    auto result = systems::DeathSystem(registry, db);

    EXPECT_FALSE(registry.Storage<component::HealthComponent>().Has(a));
    EXPECT_FALSE(registry.Storage<component::HealthComponent>().Has(b));
    EXPECT_FALSE(registry.Storage<component::HealthComponent>().Has(c));
    EXPECT_EQ(result.events.size(), 3U);
}

TEST(DeathSystem, DropsCarriedMoneyAsACurrencyItemOnDeath) {
    domain::Registry registry;
    const auto db = MakeTestDatabase();
    auto id = MakeCreature(registry, 0, true);
    registry.Storage<component::PurseComponent>().Add(id, {.money = 17});

    systems::DeathSystem(registry, db);

    auto piles = registry.View<game_item::CurrencyComponent, Location>();
    ASSERT_EQ(piles.size(), 1U);
    EXPECT_EQ(registry.GetRef<game_item::CurrencyComponent>(piles[0]).amount, 17);
    EXPECT_EQ(registry.GetRef<Location>(piles[0]).coord, (Vector2Int{.x = 1, .y = 1}));
}

TEST(DeathSystem, EmptyPurseDropsNothing) {
    domain::Registry registry;
    const auto db = MakeTestDatabase();
    auto id = MakeCreature(registry, 0, true);
    registry.Storage<component::PurseComponent>().Add(id, {.money = 0});

    systems::DeathSystem(registry, db);

    EXPECT_TRUE(registry.View<game_item::CurrencyComponent>().empty());
}
