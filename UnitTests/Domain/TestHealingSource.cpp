#include "Components.h"
#include "EntityFactory.h"
#include "EntityStorage.h"
#include "Location.h"
#include "Map.h"
#include "Registry.h"
#include "Support/Fixtures.h"
#include "Support/MakeRegistry.h"
#include "Systems.h"
#include "Vector2Int.h"
#include <gtest/gtest.h>

namespace {
const Location kSourceTile = {.coord = {.x = 6, .y = 5}, .map_level = 0};
const Location kMoverTile = {.coord = {.x = 5, .y = 5}, .map_level = 0};

entity::EntityId MakeSourceAhead(domain::Registry &registry, int amount) {
    return entity::CreateHealingSource(registry, amount, kSourceTile);
}

} // namespace

TEST(HealingSource, HealsAndConsumes) {
    auto registry = MakeTestRegistry();
    auto id = MakePlayerMoverAt(registry, kMoverTile, 5, 10);
    auto source_id = MakeSourceAhead(registry, 7);
    registry.Storage<component::IntentToMove>().Add(id, {.dcoord = Vector2Int::Right()});

    systems::MovementSystem(registry);

    ASSERT_TRUE(registry.Storage<component::IntentToHeal>().Has(id));
    EXPECT_EQ(registry.Storage<component::IntentToHeal>().Get(id)->amount, 7);
    EXPECT_FALSE(registry.Storage<map::HealingSourceComponent>().Has(source_id));
    EXPECT_FALSE(registry.Storage<Location>().Has(source_id));
}

TEST(HealingSource, MonsterLeavesIt) {
    auto registry = MakeTestRegistry();
    auto id = MakeCreatureAt(registry, kMoverTile, 5, true);
    auto source_id = MakeSourceAhead(registry, 7);
    registry.Storage<component::IntentToMove>().Add(id, {.dcoord = Vector2Int::Right()});

    systems::MovementSystem(registry);

    EXPECT_FALSE(registry.Storage<component::IntentToHeal>().Has(id));
    EXPECT_TRUE(registry.Storage<map::HealingSourceComponent>().Has(source_id));
}

TEST(HealingSource, OneShot) {
    auto registry = MakeTestRegistry();
    auto id = MakePlayerMoverAt(registry, kMoverTile, 5, 10);
    MakeSourceAhead(registry, 7);

    registry.Storage<component::IntentToMove>().Add(id, {.dcoord = Vector2Int::Right()});
    systems::MovementSystem(registry);
    ASSERT_TRUE(registry.Storage<component::IntentToHeal>().Has(id));
    registry.Storage<component::IntentToHeal>().Clear();

    registry.Storage<component::IntentToMove>().Add(id, {.dcoord = Vector2Int::Left()});
    systems::MovementSystem(registry);
    registry.Storage<component::IntentToMove>().Add(id, {.dcoord = Vector2Int::Right()});
    systems::MovementSystem(registry);

    EXPECT_FALSE(registry.Storage<component::IntentToHeal>().Has(id));
}
