#include "Components.h"
#include "EntityStorage.h"
#include "Event.h"
#include "Hero.h"
#include "Registry.h"
#include "Support/Fixtures.h"
#include "Systems.h"
#include "Tables.h"
#include <cstddef>
#include <gtest/gtest.h>

namespace {
entity::EntityId MakeFighter(domain::Registry &registry, int str) {
    auto id = registry.entities.CreateEntity();
    registry.Storage<component::AttributesComponent>().Add(id, WithStr(str));
    return id;
}
} // namespace

TEST(MeleeSystem, StampsDamageOnTargetNotAttacker) {
    domain::Registry registry;
    auto attacker = MakeFighter(registry, 20);
    auto target = MakeFighter(registry, 4);
    registry.Storage<component::IntentToMelee>().Add(attacker, {.target_entity_id = target});

    systems::MeleeSystem(registry);

    EXPECT_TRUE(registry.Storage<component::IntentToTakeDamage>().Has(target));
    EXPECT_FALSE(registry.Storage<component::IntentToTakeDamage>().Has(attacker));
    EXPECT_FALSE(registry.Storage<component::IntentToMelee>().Has(attacker));
}

TEST(MeleeSystem, TwoAttackersOnOneTargetSumDamage) {
    domain::Registry registry;
    auto attacker1 = MakeFighter(registry, 0);
    auto attacker2 = MakeFighter(registry, 0);
    auto target = MakeFighter(registry, 0);
    registry.Storage<component::IntentToMelee>().Add(attacker1, {.target_entity_id = target});
    registry.Storage<component::IntentToMelee>().Add(attacker2, {.target_entity_id = target});

    systems::MeleeSystem(registry);

    ASSERT_TRUE(registry.Storage<component::IntentToTakeDamage>().Has(target));
    auto total_damage = registry.Storage<component::IntentToTakeDamage>().Get(target)->amount;
    EXPECT_GE(total_damage, 2);
    EXPECT_LE(total_damage, 4);
}

TEST(MeleeSystem, SkilledAttackerHitsUnskilledDefender) {
    domain::Registry registry;
    auto attacker = MakeFighter(registry, 20);
    registry.Storage<hero::HeroSkills>().Add(attacker, {.skills = {100, 0, 0, 0, 0}});
    auto target = MakeFighter(registry, 4);
    registry.Storage<component::IntentToMelee>().Add(attacker, {.target_entity_id = target});

    auto result = systems::MeleeSystem(registry);

    ASSERT_TRUE(registry.Storage<component::IntentToTakeDamage>().Has(target));
    ASSERT_EQ(result.events.size(), 1U);
    EXPECT_EQ(result.events.front().type, event::Type::GotDamage);
}

TEST(MeleeSystem, UnskilledHandWeaponAlwaysMisses) {
    domain::Registry registry;
    auto attacker = MakeFighter(registry, 20);
    registry.Storage<hero::HeroSkills>().Add(attacker, {.skills = {0, 0, 0, 0, 0}});
    auto target = MakeFighter(registry, 4);
    registry.Storage<component::IntentToMelee>().Add(attacker, {.target_entity_id = target});

    auto result = systems::MeleeSystem(registry);

    EXPECT_FALSE(registry.Storage<component::IntentToTakeDamage>().Has(target));
    ASSERT_EQ(result.events.size(), 1U);
    EXPECT_EQ(result.events.front().type, event::Type::Missed);
}

TEST(MeleeSystem, DefenderWithMaxDefenceDodgesAGuaranteedHit) {
    domain::Registry registry;
    auto attacker = MakeFighter(registry, 20);
    registry.Storage<hero::HeroSkills>().Add(attacker, {.skills = {100, 0, 0, 0, 0}});
    auto target = MakeFighter(registry, 4);
    registry.Storage<hero::HeroSkills>().Add(target, {.skills = {0, 0, 100, 0, 0}});
    registry.Storage<component::IntentToMelee>().Add(attacker, {.target_entity_id = target});

    auto result = systems::MeleeSystem(registry);

    EXPECT_FALSE(registry.Storage<component::IntentToTakeDamage>().Has(target));
    ASSERT_EQ(result.events.size(), 1U);
    EXPECT_EQ(result.events.front().type, event::Type::Avoided);
}

// Statistical
TEST(MeleeSystem, RepeatedHitsEventuallyGrowHandWeaponSkill) {
    domain::Registry registry;
    auto attacker = MakeFighter(registry, 20);
    registry.Storage<hero::HeroSkills>().Add(attacker, {.skills = {90, 0, 0, 0, 0}});
    auto target = MakeFighter(registry, 4);

    for (int i = 0; i < 1000; i++) {
        registry.Storage<component::IntentToMelee>().Add(attacker, {.target_entity_id = target});
        systems::MeleeSystem(registry);
    }

    EXPECT_GT(registry.Storage<hero::HeroSkills>()
                  .Get(attacker)
                  ->skills[static_cast<std::size_t>(hero::Skill::HandWeapon)],
              90);
}

TEST(MeleeSystem, MissingAttributesStampsNothingButConsumes) {
    domain::Registry registry;
    auto attacker = registry.entities.CreateEntity();
    auto target = MakeFighter(registry, 4);
    registry.Storage<component::IntentToMelee>().Add(attacker, {.target_entity_id = target});

    systems::MeleeSystem(registry);

    EXPECT_FALSE(registry.Storage<component::IntentToTakeDamage>().Has(target));
    EXPECT_FALSE(registry.Storage<component::IntentToMelee>().Has(attacker));
}
