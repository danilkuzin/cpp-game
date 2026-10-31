#include "Components.h"
#include "EffectSystems.h"
#include "EntityStorage.h"
#include "Event.h"
#include "GameDatabase.h"
#include "Location.h"
#include "Registry.h"
#include "RegistryQueries.h"
#include "Spell.h"
#include "Support/Fixtures.h"
#include "Support/MakeRegistry.h"
#include "Systems.h"
#include "Vector2Int.h"
#include <gtest/gtest.h>
#include <optional>

namespace {
entity::EntityId MakeCasterAt(domain::Registry &registry, Location loc, int mana) {
    auto id = MakeEntityAt(registry, loc);
    registry.Storage<component::PlayerTag>().Add(id, {});
    registry.Storage<component::ManaComponent>().Add(id, {.current = mana, .max = mana});
    return id;
}

entity::EntityId MakeEnemyAt(domain::Registry &registry, Location loc) {
    auto id = MakeEntityAt(registry, loc);
    registry.Storage<component::MonsterTag>().Add(id, {});
    return id;
}

database::GameDatabase MakeSpellDatabase() {
    database::GameDatabase db;
    db.AddSpell({.id = "fireball",
                 .name = "Fireball",
                 .mana_cost = spell::ManaCostComponent{.amount = 1},
                 .damage = spell::DamageComponent{.amount = 4},
                 .healing = std::nullopt});
    db.AddSpell({.id = "healing_light",
                 .name = "Healing Light",
                 .mana_cost = spell::ManaCostComponent{.amount = 1},
                 .damage = std::nullopt,
                 .healing = spell::HealingComponent{.amount = 4}});
    return db;
}
} // namespace

TEST(SpellSystem, FireballOnEnemyInRangeStampsDamageAndSpendsMana) {
    auto registry = MakeTestRegistry();
    auto db = MakeSpellDatabase();
    auto caster = MakeCasterAt(registry, {.coord = {.x = 5, .y = 5}, .map_level = 0}, /*mana=*/3);
    auto enemy = MakeEnemyAt(registry, {.coord = {.x = 7, .y = 5}, .map_level = 0});
    registry.Storage<component::IntentToCastSpell>().Add(
        caster, {.spell_id = "fireball", .costs_mana = true, .direction = Vector2Int::Right()});

    systems::SpellSystem(registry, db);

    ASSERT_TRUE(registry.Storage<component::IntentToTakeDamage>().Has(enemy));
    EXPECT_EQ(registry.Storage<component::IntentToTakeDamage>().Get(enemy)->amount, 4);
    EXPECT_EQ(registry.Storage<component::ManaComponent>().Get(caster)->current, 2);
    EXPECT_FALSE(registry.Storage<component::IntentToCastSpell>().Has(caster));
}

TEST(SpellSystem, NoManaEmitsEventAndStampsNothing) {
    auto registry = MakeTestRegistry();
    auto db = MakeSpellDatabase();
    auto caster = MakeCasterAt(registry, {.coord = {.x = 5, .y = 5}, .map_level = 0}, /*mana=*/0);
    auto enemy = MakeEnemyAt(registry, {.coord = {.x = 7, .y = 5}, .map_level = 0});
    registry.Storage<component::IntentToCastSpell>().Add(
        caster, {.spell_id = "fireball", .costs_mana = true, .direction = Vector2Int::Right()});

    auto result = systems::SpellSystem(registry, db);

    ASSERT_EQ(result.events.size(), 1U);
    EXPECT_EQ(result.events.front().type, event::Type::NoMana);
    EXPECT_FALSE(registry.Storage<component::IntentToTakeDamage>().Has(enemy));
    EXPECT_EQ(registry.Storage<component::ManaComponent>().Get(caster)->current, 0);
    EXPECT_FALSE(registry.Storage<component::IntentToCastSpell>().Has(caster));
}

TEST(SpellSystem, HealingLightStampsIntentToHealOnCaster) {
    auto registry = MakeTestRegistry();
    auto db = MakeSpellDatabase();
    auto caster = MakeCasterAt(registry, {.coord = {.x = 5, .y = 5}, .map_level = 0}, /*mana=*/3);
    registry.Storage<component::IntentToCastSpell>().Add(
        caster,
        {.spell_id = "healing_light", .costs_mana = true, .direction = Vector2Int::Right()});

    systems::SpellSystem(registry, db);

    ASSERT_TRUE(registry.Storage<component::IntentToHeal>().Has(caster));
    EXPECT_EQ(registry.Storage<component::IntentToHeal>().Get(caster)->amount, 4);
    EXPECT_EQ(registry.Storage<component::ManaComponent>().Get(caster)->current, 2);
}

TEST(SpellSystem, NoDirectionAutoTargetsNearestEnemyAcrossTheFourCardinalRays) {
    auto registry = MakeTestRegistry();
    auto db = MakeSpellDatabase();
    auto caster = MakeCasterAt(registry, {.coord = {.x = 5, .y = 5}, .map_level = 0}, /*mana=*/3);
    auto far_enemy = MakeEnemyAt(registry, {.coord = {.x = 5, .y = 1}, .map_level = 0});
    auto near_enemy = MakeEnemyAt(registry, {.coord = {.x = 7, .y = 5}, .map_level = 0});
    registry.Storage<component::IntentToCastSpell>().Add(
        caster, {.spell_id = "fireball", .costs_mana = true, .direction = std::nullopt});

    systems::SpellSystem(registry, db);

    EXPECT_TRUE(registry.Storage<component::IntentToTakeDamage>().Has(near_enemy));
    EXPECT_FALSE(registry.Storage<component::IntentToTakeDamage>().Has(far_enemy));
}

TEST(SpellSystem, NoDirectionAndNoEnemyInRangeStampsNothing) {
    auto registry = MakeTestRegistry();
    auto db = MakeSpellDatabase();
    auto caster = MakeCasterAt(registry, {.coord = {.x = 5, .y = 5}, .map_level = 0}, /*mana=*/3);
    registry.Storage<component::IntentToCastSpell>().Add(
        caster, {.spell_id = "fireball", .costs_mana = true, .direction = std::nullopt});

    auto result = systems::SpellSystem(registry, db);

    EXPECT_TRUE(result.events.empty());
    EXPECT_EQ(registry.Storage<component::ManaComponent>().Get(caster)->current, 2);
}

TEST(SpellSystem, HealingLightResolvedThroughHealSystemRestoresCasterHp) {
    auto registry = MakeTestRegistry();
    auto db = MakeSpellDatabase();
    auto caster = MakeCasterAt(registry, {.coord = {.x = 5, .y = 5}, .map_level = 0}, /*mana=*/3);
    registry.Storage<component::HealthComponent>().Add(caster, {.current = 5, .max = 10});
    registry.Storage<component::IntentToCastSpell>().Add(
        caster,
        {.spell_id = "healing_light", .costs_mana = true, .direction = Vector2Int::Right()});

    systems::SpellSystem(registry, db);
    systems::HealSystem(registry);

    EXPECT_EQ(registry.Storage<component::HealthComponent>().Get(caster)->current, 9);
}

TEST(SpellSystem, SpellRayTilesStopAtAWall) {
    auto registry = MakeTestRegistry();
    auto caster = MakeCasterAt(registry, {.coord = {.x = 5, .y = 5}, .map_level = 0}, /*mana=*/3);
    registry.maps[0].At({.x = 7, .y = 5}).terrain_id = 4;

    const auto tiles = systems::SpellRayTiles(registry, caster, Vector2Int::Right());

    ASSERT_EQ(tiles.size(), 1U);
    EXPECT_EQ(tiles[0], (Vector2Int{.x = 6, .y = 5}));
}
