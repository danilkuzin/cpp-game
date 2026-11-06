#include "Components.h"
#include "EntityStorage.h"
#include "Event.h"
#include "GameItem.h"
#include "Hero.h"
#include "Location.h"
#include "Registry.h"
#include "Support/Fixtures.h"
#include "Support/MakeRegistry.h"
#include "Systems.h"
#include "Vector2Int.h"
#include <gtest/gtest.h>

namespace {
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
entity::EntityId EquipCrossbow(domain::Registry &registry, entity::EntityId owner, int range,
                               int ammo) {
    auto weapon_id = registry.entities.CreateEntity();
    registry.Storage<game_item::OwnershipComponent>().Add(weapon_id, {.owner = owner});
    registry.Storage<game_item::EquippedComponent>().Add(weapon_id,
                                                         {.slot = game_item::EquipSlot::MainHand});
    registry.Storage<game_item::RangedWeaponComponent>().Add(
        weapon_id, {.damage = {.count = 1, .sides = 4}, .max_ammo = 10, .range = range});
    registry.Storage<game_item::LoadedAmmoComponent>().Add(weapon_id, {.amount = ammo});
    return weapon_id;
}

entity::EntityId MakeShooterAt(domain::Registry &registry, Location loc) {
    auto id = MakeEntityAt(registry, loc);
    registry.Storage<component::PlayerTag>().Add(id, {});
    return id;
}

entity::EntityId MakeTargetAt(domain::Registry &registry, Location loc) {
    return MakeCreatureAt(registry, loc, 10, true);
}
} // namespace

TEST(RangedSystem, EnemyInRangeAlongDirectionTakesDamageAndAmmoDrops) {
    auto registry = MakeTestRegistry();
    auto shooter = MakeShooterAt(registry, {.coord = {.x = 5, .y = 5}, .map_level = 0});
    auto weapon_id = EquipCrossbow(registry, shooter, 5, 3);
    auto enemy = MakeTargetAt(registry, {.coord = {.x = 7, .y = 5}, .map_level = 0});
    registry.Storage<component::IntentToShoot>().Add(shooter, {.direction = Vector2Int::Right()});

    systems::RangedSystem(registry);

    ASSERT_TRUE(registry.Storage<component::IntentToTakeDamage>().Has(enemy));
    auto damage = registry.Storage<component::IntentToTakeDamage>().Get(enemy)->amount;
    EXPECT_GE(damage, 1);
    EXPECT_LE(damage, 4);
    EXPECT_EQ(registry.Storage<game_item::LoadedAmmoComponent>().Get(weapon_id)->amount, 2);
    EXPECT_FALSE(registry.Storage<component::IntentToShoot>().Has(shooter));
}

TEST(RangedSystem, WallBetweenShooterAndEnemyStopsTheShot) {
    auto registry = MakeTestRegistry();
    auto shooter = MakeShooterAt(registry, {.coord = {.x = 5, .y = 5}, .map_level = 0});
    auto weapon_id = EquipCrossbow(registry, shooter, 5, 3);
    auto enemy = MakeTargetAt(registry, {.coord = {.x = 7, .y = 5}, .map_level = 0});
    registry.maps[0].At({.x = 6, .y = 5}).terrain_id = 4;
    registry.Storage<component::IntentToShoot>().Add(shooter, {.direction = Vector2Int::Right()});

    systems::RangedSystem(registry);

    EXPECT_FALSE(registry.Storage<component::IntentToTakeDamage>().Has(enemy));
    EXPECT_EQ(registry.Storage<game_item::LoadedAmmoComponent>().Get(weapon_id)->amount, 2);
}

TEST(RangedSystem, NoTargetInRangeSpendsAmmoButStampsNothing) {
    auto registry = MakeTestRegistry();
    auto shooter = MakeShooterAt(registry, {.coord = {.x = 5, .y = 5}, .map_level = 0});
    auto weapon_id = EquipCrossbow(registry, shooter, 5, 3);
    registry.Storage<component::IntentToShoot>().Add(shooter, {.direction = Vector2Int::Right()});

    systems::RangedSystem(registry);

    EXPECT_TRUE(registry.Storage<component::IntentToTakeDamage>().Empty());
    EXPECT_EQ(registry.Storage<game_item::LoadedAmmoComponent>().Get(weapon_id)->amount, 2);
}

TEST(RangedSystem, NoWeaponEmitsEventAndStampsNothing) {
    auto registry = MakeTestRegistry();
    auto shooter = MakeShooterAt(registry, {.coord = {.x = 5, .y = 5}, .map_level = 0});
    registry.Storage<component::IntentToShoot>().Add(shooter, {.direction = Vector2Int::Right()});

    auto result = systems::RangedSystem(registry);

    ASSERT_EQ(result.events.size(), 1U);
    EXPECT_EQ(result.events.front().type, event::Type::NoWeapon);
    EXPECT_TRUE(registry.Storage<component::IntentToTakeDamage>().Empty());
    EXPECT_FALSE(registry.Storage<component::IntentToShoot>().Has(shooter));
}

TEST(RangedSystem, NoAmmoEmitsEventAndStampsNothing) {
    auto registry = MakeTestRegistry();
    auto shooter = MakeShooterAt(registry, {.coord = {.x = 5, .y = 5}, .map_level = 0});
    auto weapon_id = EquipCrossbow(registry, shooter, 5, 0);
    auto enemy = MakeTargetAt(registry, {.coord = {.x = 7, .y = 5}, .map_level = 0});
    registry.Storage<component::IntentToShoot>().Add(shooter, {.direction = Vector2Int::Right()});

    auto result = systems::RangedSystem(registry);

    ASSERT_EQ(result.events.size(), 1U);
    EXPECT_EQ(result.events.front().type, event::Type::NoAmmo);
    EXPECT_FALSE(registry.Storage<component::IntentToTakeDamage>().Has(enemy));
    EXPECT_EQ(registry.Storage<game_item::LoadedAmmoComponent>().Get(weapon_id)->amount, 0);
}

TEST(RangedSystem, SkilledShooterHitsUnskilledDefender) {
    auto registry = MakeTestRegistry();
    auto shooter = MakeShooterAt(registry, {.coord = {.x = 5, .y = 5}, .map_level = 0});
    registry.Storage<hero::HeroSkills>().Add(shooter, {.skills = {0, 0, 0, 100, 0}});
    EquipCrossbow(registry, shooter, 5, 3);
    auto enemy = MakeTargetAt(registry, {.coord = {.x = 7, .y = 5}, .map_level = 0});
    registry.Storage<component::IntentToShoot>().Add(shooter, {.direction = Vector2Int::Right()});

    auto result = systems::RangedSystem(registry);

    ASSERT_TRUE(registry.Storage<component::IntentToTakeDamage>().Has(enemy));
    ASSERT_EQ(result.events.size(), 1U);
    EXPECT_EQ(result.events.front().type, event::Type::GotDamage);
}

TEST(RangedSystem, UnskilledRangedWeaponAlwaysMisses) {
    auto registry = MakeTestRegistry();
    auto shooter = MakeShooterAt(registry, {.coord = {.x = 5, .y = 5}, .map_level = 0});
    registry.Storage<hero::HeroSkills>().Add(shooter, {.skills = {0, 0, 0, 0, 0}});
    auto weapon_id = EquipCrossbow(registry, shooter, 5, 3);
    auto enemy = MakeTargetAt(registry, {.coord = {.x = 7, .y = 5}, .map_level = 0});
    registry.Storage<component::IntentToShoot>().Add(shooter, {.direction = Vector2Int::Right()});

    auto result = systems::RangedSystem(registry);

    EXPECT_FALSE(registry.Storage<component::IntentToTakeDamage>().Has(enemy));
    ASSERT_EQ(result.events.size(), 1U);
    EXPECT_EQ(result.events.front().type, event::Type::Missed);
    EXPECT_EQ(registry.Storage<game_item::LoadedAmmoComponent>().Get(weapon_id)->amount, 2);
}

TEST(RangedSystem, DefenderWithMaxDefenceDodgesAGuaranteedHit) {
    auto registry = MakeTestRegistry();
    auto shooter = MakeShooterAt(registry, {.coord = {.x = 5, .y = 5}, .map_level = 0});
    registry.Storage<hero::HeroSkills>().Add(shooter, {.skills = {0, 0, 0, 100, 0}});
    EquipCrossbow(registry, shooter, 5, 3);
    auto enemy = MakeTargetAt(registry, {.coord = {.x = 7, .y = 5}, .map_level = 0});
    registry.Storage<hero::HeroSkills>().Add(enemy, {.skills = {0, 0, 100, 0, 0}});
    registry.Storage<component::IntentToShoot>().Add(shooter, {.direction = Vector2Int::Right()});

    auto result = systems::RangedSystem(registry);

    EXPECT_FALSE(registry.Storage<component::IntentToTakeDamage>().Has(enemy));
    ASSERT_EQ(result.events.size(), 1U);
    EXPECT_EQ(result.events.front().type, event::Type::Avoided);
}
