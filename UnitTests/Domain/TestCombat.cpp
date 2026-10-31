#include "Combat.h"
#include "Components.h"
#include "GameItem.h"
#include "Registry.h"
#include "Support/Fixtures.h"
#include <gtest/gtest.h>

namespace {
game_item::MeleeWeaponComponent MakeWeapon(int dice, int sides) {
    return {.damage = {.count = dice, .sides = sides}};
}

game_item::RangedWeaponComponent MakeRangedWeapon(int dice, int sides, int range = 5) {
    return {.damage = {.count = dice, .sides = sides}, .max_ammo = 10, .range = range};
}

} // namespace

TEST(ResolveMelee, DamageNeverNegativeEvenAgainstOverwhelmingMitigation) {
    auto damage = combat::ResolveMelee(WithStr(0), combat::kUnarmedMelee, WithStr(40), 100);

    EXPECT_EQ(damage, 0);
}

TEST(ResolveMelee, WeaponDiceFallWithinExpectedRange) {
    auto weapon = MakeWeapon(3, 6);

    auto damage = combat::ResolveMelee(WithStr(0), weapon, WithStr(0), 0);

    EXPECT_GE(damage, 3);
    EXPECT_LE(damage, 18);
}

TEST(ResolveMelee, ArmorReducesIncomingDamageToZeroWhenOverwhelming) {
    auto weapon = MakeWeapon(3, 6);

    auto damage = combat::ResolveMelee(WithStr(0), weapon, WithStr(0), 100);

    EXPECT_EQ(damage, 0);
}

TEST(ResolveMelee, ArmedAttackerOutDamagesUnarmedAttacker) {
    auto armed_damage = combat::ResolveMelee(WithStr(0), MakeWeapon(5, 6), WithStr(0), 0);
    auto unarmed_damage = combat::ResolveMelee(WithStr(0), combat::kUnarmedMelee, WithStr(0), 0);

    EXPECT_GT(armed_damage, unarmed_damage);
}

TEST(EffectiveMelee, FallsBackToUnarmedWhenNothingEquipped) {
    domain::Registry registry;
    auto id = registry.entities.CreateEntity();

    auto weapon = combat::EffectiveMelee(registry, id);

    EXPECT_EQ(weapon.damage.count, combat::kUnarmedMelee.damage.count);
    EXPECT_EQ(weapon.damage.sides, combat::kUnarmedMelee.damage.sides);
}

TEST(EffectiveMelee, ReturnsEquippedWeapon) {
    domain::Registry registry;
    auto id = registry.entities.CreateEntity();
    auto sword = MakeEquippedItem(registry, id, game_item::EquipSlot::MainHand);
    registry.Storage<game_item::MeleeWeaponComponent>().Add(sword, MakeWeapon(2, 6));

    auto weapon = combat::EffectiveMelee(registry, id);

    EXPECT_EQ(weapon.damage.count, 2);
    EXPECT_EQ(weapon.damage.sides, 6);
}

TEST(EffectiveMelee, ResolvesTheSameWayForHeroAndMonster) {
    domain::Registry registry;
    auto hero = registry.entities.CreateEntity();
    registry.Storage<component::PlayerTag>().Add(hero, {});
    auto sword = MakeEquippedItem(registry, hero, game_item::EquipSlot::MainHand);
    registry.Storage<game_item::MeleeWeaponComponent>().Add(sword, MakeWeapon(2, 6));

    auto monster = registry.entities.CreateEntity();
    registry.Storage<component::MonsterTag>().Add(monster, {});

    EXPECT_EQ(combat::EffectiveMelee(registry, hero).damage.count, 2);
    EXPECT_EQ(combat::EffectiveMelee(registry, monster).damage.count,
              combat::kUnarmedMelee.damage.count);
}

TEST(EffectiveMelee, UnarmedDiceSidesScaleWithAttackerStr) {
    domain::Registry registry;
    auto rat = registry.entities.CreateEntity();
    registry.Storage<component::AttributesComponent>().Add(rat, WithStr(1));
    auto troll = registry.entities.CreateEntity();
    registry.Storage<component::AttributesComponent>().Add(troll, WithStr(7));

    EXPECT_EQ(combat::EffectiveMelee(registry, rat).damage.sides, combat::kUnarmedBaseSides + 1);
    EXPECT_EQ(combat::EffectiveMelee(registry, troll).damage.sides, combat::kUnarmedBaseSides + 7);
}

TEST(ResolveMelee, HighStrUnarmedAttackerOutDamagesLowStrUnarmedAttacker) {
    domain::Registry registry;
    auto rat = registry.entities.CreateEntity();
    registry.Storage<component::AttributesComponent>().Add(rat, WithStr(1));
    auto troll = registry.entities.CreateEntity();
    registry.Storage<component::AttributesComponent>().Add(troll, WithStr(7));

    auto rat_damage =
        combat::ResolveMelee(WithStr(1), combat::EffectiveMelee(registry, rat), WithStr(0), 0);
    auto troll_damage =
        combat::ResolveMelee(WithStr(7), combat::EffectiveMelee(registry, troll), WithStr(0), 0);

    EXPECT_GT(troll_damage, rat_damage);
}

TEST(EffectiveArmor, SumsEquippedArmorPieces) {
    domain::Registry registry;
    auto id = registry.entities.CreateEntity();
    auto chest = MakeEquippedItem(registry, id, game_item::EquipSlot::Chest);
    registry.Storage<game_item::ArmorComponent>().Add(chest, {.defence = 3});
    auto helm = MakeEquippedItem(registry, id, game_item::EquipSlot::Head);
    registry.Storage<game_item::ArmorComponent>().Add(helm, {.defence = 2});

    EXPECT_EQ(combat::EffectiveArmor(registry, id), 5);
}

TEST(EffectiveArmor, IgnoresUnequippedArmor) {
    domain::Registry registry;
    auto id = registry.entities.CreateEntity();
    auto chest = registry.entities.CreateEntity();
    registry.Storage<game_item::OwnershipComponent>().Add(chest, {.owner = id});
    registry.Storage<game_item::ArmorComponent>().Add(chest, {.defence = 3});

    EXPECT_EQ(combat::EffectiveArmor(registry, id), 0);
}

TEST(ResolveRanged, DamageNeverNegativeEvenAgainstOverwhelmingMitigation) {
    auto damage = combat::ResolveRanged(WithDex(0), MakeRangedWeapon(1, 4), WithStr(40), 100);

    EXPECT_EQ(damage, 0);
}

TEST(ResolveRanged, WeaponDiceFallWithinExpectedRangePlusDexModifier) {
    auto weapon = MakeRangedWeapon(3, 6);

    auto damage = combat::ResolveRanged(WithDex(0), weapon, WithStr(0), 0);

    EXPECT_GE(damage, 3);
    EXPECT_LE(damage, 18);
}

TEST(ResolveRanged, HighDexOutDamagesLowDex) {
    auto high_dex_damage =
        combat::ResolveRanged(WithDex(40), MakeRangedWeapon(1, 2), WithStr(0), 0);
    auto low_dex_damage = combat::ResolveRanged(WithDex(0), MakeRangedWeapon(1, 2), WithStr(0), 0);

    EXPECT_GT(high_dex_damage, low_dex_damage);
}

TEST(EffectiveRanged, ReturnsNulloptWhenNothingEquipped) {
    domain::Registry registry;
    auto id = registry.entities.CreateEntity();

    EXPECT_FALSE(combat::EffectiveRanged(registry, id).has_value());
}

TEST(EffectiveRanged, ReturnsEquippedWeapon) {
    domain::Registry registry;
    auto id = registry.entities.CreateEntity();
    auto crossbow = MakeEquippedItem(registry, id, game_item::EquipSlot::MainHand);
    registry.Storage<game_item::RangedWeaponComponent>().Add(crossbow, MakeRangedWeapon(1, 4, 5));

    auto weapon = combat::EffectiveRanged(registry, id);

    ASSERT_TRUE(weapon.has_value());
    if (weapon.has_value()) {
        EXPECT_EQ(weapon->damage.count, 1);
        EXPECT_EQ(weapon->damage.sides, 4);
        EXPECT_EQ(weapon->range, 5);
    }
}

TEST(EffectiveRangedWeaponId, ReturnsTheEquippedWeaponsOwnId) {
    domain::Registry registry;
    auto id = registry.entities.CreateEntity();
    auto crossbow = MakeEquippedItem(registry, id, game_item::EquipSlot::MainHand);
    registry.Storage<game_item::RangedWeaponComponent>().Add(crossbow, MakeRangedWeapon(1, 4));

    auto weapon_id = combat::EffectiveRangedWeaponId(registry, id);

    ASSERT_TRUE(weapon_id.has_value());
    if (weapon_id.has_value()) {
        EXPECT_EQ(*weapon_id, crossbow);
    }
}

TEST(EffectiveRanged, IgnoresEquippedMeleeWeapon) {
    domain::Registry registry;
    auto id = registry.entities.CreateEntity();
    auto axe = MakeEquippedItem(registry, id, game_item::EquipSlot::MainHand);
    registry.Storage<game_item::MeleeWeaponComponent>().Add(axe, MakeWeapon(1, 6));

    EXPECT_FALSE(combat::EffectiveRanged(registry, id).has_value());
}
