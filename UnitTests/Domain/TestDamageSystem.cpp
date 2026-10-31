#include "Components.h"
#include "EffectSystems.h"
#include "Event.h"
#include "Registry.h"
#include "Systems.h"
#include <gtest/gtest.h>
#include <optional>

TEST(DamageSystem, ReducesHealth) {
    domain::Registry registry;
    auto id = registry.entities.CreateEntity();
    registry.Storage<component::HealthComponent>().Add(id, {.current = 10, .max = 10});
    registry.Storage<component::IntentToTakeDamage>().Add(id, {.amount = 3});

    systems::DamageSystem(registry);

    EXPECT_EQ(registry.Storage<component::HealthComponent>().Get(id)->current, 7);
    systems::DeathSystem(registry, database::GameDatabase{});
    EXPECT_FALSE(registry.Storage<component::IntentToTakeDamage>().Has(id));
}

TEST(DamageSystem, ClampsAtZeroNeverNegative) {
    domain::Registry registry;
    auto id = registry.entities.CreateEntity();
    registry.Storage<component::HealthComponent>().Add(id, {.current = 2, .max = 10});
    registry.Storage<component::IntentToTakeDamage>().Add(id, {.amount = 5});

    systems::DamageSystem(registry);

    EXPECT_EQ(registry.Storage<component::HealthComponent>().Get(id)->current, 0);
}

TEST(DamageSystem, ConsumesIntentEvenWithoutHealth) {
    domain::Registry registry;
    auto id = registry.entities.CreateEntity();
    registry.Storage<component::IntentToTakeDamage>().Add(id, {.amount = 5});

    systems::DamageSystem(registry);
    systems::DeathSystem(registry, database::GameDatabase{});

    EXPECT_FALSE(registry.Storage<component::IntentToTakeDamage>().Has(id));
}

TEST(StampDamage, CreatesTheIntentAndEmitsGotDamage) {
    domain::Registry registry;
    auto attacker = registry.entities.CreateEntity();
    auto target = registry.entities.CreateEntity();
    event::Result result{};

    systems::StampDamage(registry, result, attacker, target, 4);

    const auto *intent = registry.Storage<component::IntentToTakeDamage>().Get(target);
    ASSERT_NE(intent, nullptr);
    EXPECT_EQ(intent->amount, 4);
    EXPECT_EQ(intent->attacker, attacker);
    ASSERT_EQ(result.events.size(), 1U);
    EXPECT_EQ(result.events[0].type, event::Type::GotDamage);
    EXPECT_EQ(result.events[0].defender, target);
    EXPECT_EQ(result.events[0].value, 4);
}

TEST(StampDamage, SecondHitAccumulatesAndTakesTheKillCredit) {
    domain::Registry registry;
    auto first = registry.entities.CreateEntity();
    auto second = registry.entities.CreateEntity();
    auto target = registry.entities.CreateEntity();
    event::Result result{};

    systems::StampDamage(registry, result, first, target, 3);
    systems::StampDamage(registry, result, second, target, 2);

    const auto *intent = registry.Storage<component::IntentToTakeDamage>().Get(target);
    ASSERT_NE(intent, nullptr);
    EXPECT_EQ(intent->amount, 5);
    EXPECT_EQ(intent->attacker, second);
    EXPECT_EQ(result.events.size(), 2U);
}

TEST(StampDamage, UnattributedDamageKeepsAnEarlierAttackersCredit) {
    domain::Registry registry;
    auto hero = registry.entities.CreateEntity();
    auto target = registry.entities.CreateEntity();
    event::Result result{};

    systems::StampDamage(registry, result, hero, target, 3);
    systems::StampDamage(registry, result, std::nullopt, target, 2, event::Type::TrapDamage);

    const auto *intent = registry.Storage<component::IntentToTakeDamage>().Get(target);
    ASSERT_NE(intent, nullptr);
    EXPECT_EQ(intent->amount, 5);
    EXPECT_EQ(intent->attacker, hero);
    EXPECT_EQ(result.events[1].type, event::Type::TrapDamage);
    EXPECT_FALSE(result.events[1].attacker.has_value());
}
