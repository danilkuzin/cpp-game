#include "Components.h"
#include "EffectSystems.h"
#include "Event.h"
#include "Registry.h"
#include <gtest/gtest.h>

TEST(HealSystem, ClampsAtMaxAndEmitsHealedEvent) {
    domain::Registry registry;
    auto id = registry.entities.CreateEntity();
    registry.Storage<component::HealthComponent>().Add(id, {.current = 5, .max = 10});
    registry.Storage<component::IntentToHeal>().Add(id, {.amount = 8});

    auto result = systems::HealSystem(registry);

    EXPECT_EQ(registry.Storage<component::HealthComponent>().Get(id)->current, 10);
    EXPECT_FALSE(registry.Storage<component::IntentToHeal>().Has(id));
    ASSERT_EQ(result.events.size(), 1U);
    EXPECT_EQ(result.events.front().type, event::Type::Healed);
    EXPECT_EQ(result.events.front().value, 8);
}

TEST(HealSystem, FullHpTargetStaysAtMax) {
    domain::Registry registry;
    auto id = registry.entities.CreateEntity();
    registry.Storage<component::HealthComponent>().Add(id, {.current = 10, .max = 10});
    registry.Storage<component::IntentToHeal>().Add(id, {.amount = 5});

    systems::HealSystem(registry);

    EXPECT_EQ(registry.Storage<component::HealthComponent>().Get(id)->current, 10);
    EXPECT_FALSE(registry.Storage<component::IntentToHeal>().Has(id));
}
