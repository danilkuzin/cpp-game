#include "Components.h"
#include "EffectSystems.h"
#include "EntityFactory.h"
#include "EntityStorage.h"
#include "Event.h"
#include "GameItem.h"
#include "Hero.h"
#include "Location.h"
#include "Map.h"
#include "Registry.h"
#include "Support/Fixtures.h"
#include "Support/MakeRegistry.h"
#include "Systems.h"
#include "Vector2Int.h"
#include <algorithm>
#include <gtest/gtest.h>

namespace {
const Location kTrapTile = {.coord = {.x = 6, .y = 5}, .map_level = 0};
const Location kMoverTile = {.coord = {.x = 5, .y = 5}, .map_level = 0};

entity::EntityId MakeTrapAhead(domain::Registry &registry, int difficulty = 0) {
    return entity::CreateTrap(registry, difficulty, kTrapTile);
}

} // namespace

TEST(TrapEncounter, SteppingOnActiveTrapStampsDamageIntentAndFiresEvent) {
    auto registry = MakeTestRegistry();

    auto id = MakeMoverAt(registry, kMoverTile, /*hp=*/3, /*max_hp=*/1);
    MakeTrapAhead(registry);
    registry.Storage<component::IntentToMove>().Add(id, {.dcoord = Vector2Int::Right()});

    auto result = systems::MovementSystem(registry);

    ASSERT_TRUE(registry.Storage<component::IntentToTakeDamage>().Has(id));
    EXPECT_EQ(registry.Storage<component::IntentToTakeDamage>().Get(id)->amount, 2);
    ASSERT_EQ(result.events.size(), 1U);
    EXPECT_EQ(result.events.front().type, event::Type::TrapDamage);
    EXPECT_EQ(result.events.front().value, 2);
}

TEST(TrapEncounter, TrapDamageIsFlooredAtCurrentHpMinusOne) {
    auto registry = MakeTestRegistry();
    auto id = MakeMoverAt(registry, kMoverTile, /*hp=*/2, /*max_hp=*/100);
    MakeTrapAhead(registry);
    registry.Storage<component::IntentToMove>().Add(id, {.dcoord = Vector2Int::Right()});

    auto result = systems::MovementSystem(registry);
    systems::DamageSystem(registry);

    ASSERT_EQ(result.events.size(), 1U);
    EXPECT_EQ(result.events.front().value, 1);
    ASSERT_TRUE(registry.Storage<component::HealthComponent>().Has(id));
    EXPECT_EQ(registry.Storage<component::HealthComponent>().Get(id)->current, 1);
}

TEST(TrapEncounter, OneHpMoverSurvivesATrapRegardlessOfTheRoll) {
    auto registry = MakeTestRegistry();
    auto id = MakeMoverAt(registry, kMoverTile, /*hp=*/1, /*max_hp=*/100);
    MakeTrapAhead(registry);
    registry.Storage<component::IntentToMove>().Add(id, {.dcoord = Vector2Int::Right()});

    auto result = systems::MovementSystem(registry);
    systems::DamageSystem(registry);

    ASSERT_EQ(result.events.size(), 1U);
    EXPECT_EQ(result.events.front().value, 0);
    ASSERT_TRUE(registry.Storage<component::HealthComponent>().Has(id));
    EXPECT_EQ(registry.Storage<component::HealthComponent>().Get(id)->current, 1);
}

TEST(TrapEncounter, TrapIsOneShot) {
    auto registry = MakeTestRegistry();
    auto id = MakeMoverAt(registry, kMoverTile, /*hp=*/100, /*max_hp=*/100);
    auto trap_id = MakeTrapAhead(registry);

    registry.Storage<component::IntentToMove>().Add(id, {.dcoord = Vector2Int::Right()});
    systems::MovementSystem(registry);
    ASSERT_TRUE(registry.Storage<component::IntentToTakeDamage>().Has(id));
    ASSERT_TRUE(registry.Storage<map::TrapComponent>().Has(trap_id));
    EXPECT_FALSE(registry.GetRef<map::TrapComponent>(trap_id).active);
    registry.Storage<component::IntentToTakeDamage>().Clear();

    registry.Storage<component::IntentToMove>().Add(id, {.dcoord = Vector2Int::Left()});
    systems::MovementSystem(registry);
    registry.Storage<component::IntentToMove>().Add(id, {.dcoord = Vector2Int::Right()});
    auto result = systems::MovementSystem(registry);

    EXPECT_FALSE(registry.Storage<component::IntentToTakeDamage>().Has(id));
    EXPECT_TRUE(result.events.empty());
}

TEST(TrapEncounter, TrapDamageAppliesThroughDamageSystem) {
    auto registry = MakeTestRegistry();
    auto id = MakeMoverAt(registry, kMoverTile, /*hp=*/100, /*max_hp=*/100);
    MakeTrapAhead(registry);
    registry.Storage<component::IntentToMove>().Add(id, {.dcoord = Vector2Int::Right()});

    auto move_result = systems::MovementSystem(registry);
    auto trap_damage = move_result.events.front().value;
    systems::DamageSystem(registry);

    EXPECT_EQ(registry.Storage<component::HealthComponent>().Get(id)->current,
              std::max(0, 100 - trap_damage));
}

TEST(TrapEncounter, WouldBeLethalTrapLeavesMoverAliveAndLootUntouched) {
    auto registry = MakeTestRegistry();
    auto id = MakeMoverAt(registry, kMoverTile, /*hp=*/1, /*max_hp=*/1);
    registry.Storage<component::MonsterTag>().Add(id, {});
    MakeTrapAhead(registry);

    auto sword = registry.entities.CreateEntity();
    registry.Storage<game_item::OwnershipComponent>().Add(sword, {.owner = id});
    registry.Storage<game_item::SizeComponent>().Add(sword, {.size = 1});

    registry.Storage<component::IntentToMove>().Add(id, {.dcoord = Vector2Int::Right()});
    systems::MovementSystem(registry);
    systems::DamageSystem(registry);
    auto death_result = systems::DeathSystem(registry, database::GameDatabase{});

    ASSERT_TRUE(registry.Storage<component::HealthComponent>().Has(id));
    EXPECT_EQ(registry.Storage<component::HealthComponent>().Get(id)->current, 1);
    EXPECT_TRUE(death_result.events.empty());
    EXPECT_TRUE(registry.Storage<game_item::OwnershipComponent>().Has(sword));
}

TEST(TrapEncounter, MoverWithSkillDisarmsTrapOnGuaranteedSuccess) {
    auto registry = MakeTestRegistry();
    auto id = MakeMoverAt(registry, kMoverTile, /*hp=*/10, /*max_hp=*/10);
    registry.Storage<hero::HeroSkills>().Add(id, {.skills = {50, 50, 50, 50, 50}});
    auto trap_id = MakeTrapAhead(registry, /*difficulty=*/-1000);
    registry.Storage<component::IntentToMove>().Add(id, {.dcoord = Vector2Int::Right()});

    auto result = systems::MovementSystem(registry);

    EXPECT_FALSE(registry.Storage<component::IntentToTakeDamage>().Has(id));
    ASSERT_EQ(result.events.size(), 1U);
    EXPECT_EQ(result.events.front().type, event::Type::TrapDisarmed);
    ASSERT_TRUE(registry.Storage<map::TrapComponent>().Has(trap_id));
    EXPECT_FALSE(registry.GetRef<map::TrapComponent>(trap_id).active);
}

TEST(TrapEncounter, MoverWithSkillTakesDamageOnGuaranteedFailure) {
    auto registry = MakeTestRegistry();
    auto id = MakeMoverAt(registry, kMoverTile, /*hp=*/100, /*max_hp=*/100);
    registry.Storage<hero::HeroSkills>().Add(id, {.skills = {50, 50, 50, 50, 50}});
    auto trap_id = MakeTrapAhead(registry, /*difficulty=*/1000);
    registry.Storage<component::IntentToMove>().Add(id, {.dcoord = Vector2Int::Right()});

    auto result = systems::MovementSystem(registry);

    ASSERT_TRUE(registry.Storage<component::IntentToTakeDamage>().Has(id));
    ASSERT_EQ(result.events.size(), 1U);
    EXPECT_EQ(result.events.front().type, event::Type::TrapDamage);
    ASSERT_TRUE(registry.Storage<map::TrapComponent>().Has(trap_id));
    EXPECT_FALSE(registry.GetRef<map::TrapComponent>(trap_id).active);
}

TEST(TrapEncounter, MoverWithoutSkillsComponentAlwaysSpringsTheTrap) {
    auto registry = MakeTestRegistry();
    auto id = MakeMoverAt(registry, kMoverTile, /*hp=*/100, /*max_hp=*/100);
    registry.Storage<component::MonsterTag>().Add(id, {});
    auto trap_id = MakeTrapAhead(registry, /*difficulty=*/-1000);
    registry.Storage<component::IntentToMove>().Add(id, {.dcoord = Vector2Int::Right()});

    auto result = systems::MovementSystem(registry);

    ASSERT_TRUE(registry.Storage<component::IntentToTakeDamage>().Has(id));
    ASSERT_EQ(result.events.size(), 1U);
    EXPECT_EQ(result.events.front().type, event::Type::TrapDamage);
    ASSERT_TRUE(registry.Storage<map::TrapComponent>().Has(trap_id));
    EXPECT_FALSE(registry.GetRef<map::TrapComponent>(trap_id).active);
}
