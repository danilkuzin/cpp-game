#include "ComponentStorage.h"
#include "Components.h"
#include "EntityFactory.h"
#include "GameDatabase.h"
#include "GameItem.h"
#include "Hero.h"
#include "Inventory.h"
#include "Location.h"
#include "Support/TestData.h"
#include "Tables.h"
#include <cstddef>
#include <gtest/gtest.h>
#include <optional>
#include <utility>

TEST(Hero, CreateHeroStampsEveryComponentAStartingHeroNeeds) {
    auto registry = domain::Registry{};

    const entity::HeroGenerationParameters hparams{
        .name = "abc", .race = hero::Race::Dwarf, .hclass = hero::HClass::Mage};
    const Location loc{.coord = {.x = 10, .y = 20}, .map_level = 3};

    auto hero_id = entity::CreateHero(registry, hparams, loc);

    EXPECT_EQ(registry.Storage<component::NameComponent>().Get(hero_id)->name, hparams.name);
    EXPECT_EQ(registry.Storage<component::VisionComponent>().Get(hero_id)->view_zone,
              hero::kVisionRadius);
    EXPECT_EQ(*registry.Storage<Location>().Get(hero_id), loc);
    EXPECT_TRUE(registry.Storage<component::PlayerTag>().Has(hero_id));
    EXPECT_FALSE(registry.Storage<component::MonsterTag>().Has(hero_id));
    EXPECT_TRUE(registry.Storage<component::RenderComponent>().Has(hero_id));
    EXPECT_TRUE(registry.Storage<component::HealthComponent>().Has(hero_id));
    auto *mana = registry.Storage<component::ManaComponent>().Get(hero_id);
    ASSERT_NE(mana, nullptr);
    EXPECT_EQ(mana->max, hero::kManaLevelTable[std::to_underlying(hero::HClass::Mage)][0]);
    EXPECT_EQ(mana->current, mana->max);

    auto *xp = registry.Storage<hero::HeroExperience>().Get(hero_id);
    ASSERT_NE(xp, nullptr);
    EXPECT_EQ(xp->xp, 0);
    EXPECT_EQ(xp->level, 1);
    EXPECT_EQ(xp->hclass, hparams.hclass);
}

TEST(Hero, CreationStampsEachSkillWithinItsClassRange) {
    auto registry = domain::Registry{};
    const entity::HeroGenerationParameters hparams{
        .name = "abc", .race = hero::Race::Human, .hclass = hero::HClass::Mage};

    auto hero_id =
        entity::CreateHero(registry, hparams, {.coord = {.x = 0, .y = 0}, .map_level = 0});

    const auto *skills = registry.Storage<hero::HeroSkills>().Get(hero_id);
    ASSERT_NE(skills, nullptr);
    auto hclass_idx = std::to_underlying(hparams.hclass);
    for (std::size_t si = 0; si < skills->skills.size(); ++si) {
        auto min = hero::kClassSkillTable[si][hclass_idx].first;
        auto increment = hero::kClassSkillTable[si][hclass_idx].second;
        EXPECT_GE(skills->skills[si], min);
        EXPECT_LE(skills->skills[si], min + increment);
    }
}

TEST(Hero, GiveStartingKitEquipsAWeaponWhenOneExistsInTheDatabase) {
    auto registry = domain::Registry{};
    database::GameDatabase db;
    auto axe = MakeItemTemplate("axe_standard", "Axe", /*size=*/2, "../axe.bmp");
    axe.equippable = game_item::EquippableComponent{.allowed_slots = game_item::EquipSlot::AnyHand};
    db.AddItem(axe);
    const entity::HeroGenerationParameters hparams{.name = "abc"};
    auto hero_id =
        entity::CreateHero(registry, hparams, {.coord = {.x = 0, .y = 0}, .map_level = 0});

    entity::GiveStartingKit(registry, db, hero_id, hero::HClass::Warrior);

    auto owned = game_item::OwnedItems(registry, hero_id);
    ASSERT_EQ(owned.size(), 1U);
    EXPECT_TRUE(registry.Storage<game_item::EquippedComponent>().Has(owned.front()));
    EXPECT_EQ(registry.Storage<game_item::EquippedComponent>().Get(owned.front())->slot,
              game_item::EquipSlot::MainHand);
}

TEST(Hero, GiveStartingKitIsANoOpWhenDatabaseHasNoWeapon) {
    auto registry = domain::Registry{};
    const database::GameDatabase db;
    const entity::HeroGenerationParameters hparams{.name = "abc"};
    auto hero_id =
        entity::CreateHero(registry, hparams, {.coord = {.x = 0, .y = 0}, .map_level = 0});

    entity::GiveStartingKit(registry, db, hero_id, hero::HClass::Warrior);

    EXPECT_TRUE(game_item::OwnedItems(registry, hero_id).empty());
}

TEST(Hero, GiveStartingKitEquipsCrossbowForRanger) {
    auto registry = domain::Registry{};
    database::GameDatabase db;
    auto crossbow = MakeItemTemplate("crossbow_standard", "CrossBow", /*size=*/2, "../cross.bmp");
    crossbow.ranged = game_item::RangedWeaponComponent{
        .damage = {.count = 1, .sides = 4}, .max_ammo = 20, .range = 5};
    crossbow.equippable =
        game_item::EquippableComponent{.allowed_slots = game_item::EquipSlot::AnyHand};
    db.AddItem(crossbow);
    const entity::HeroGenerationParameters hparams{.name = "abc", .hclass = hero::HClass::Ranger};
    auto hero_id =
        entity::CreateHero(registry, hparams, {.coord = {.x = 0, .y = 0}, .map_level = 0});

    entity::GiveStartingKit(registry, db, hero_id, hero::HClass::Ranger);

    auto owned = game_item::OwnedItems(registry, hero_id);
    ASSERT_EQ(owned.size(), 1U);
    auto weapon_id = owned.front();
    EXPECT_TRUE(registry.Storage<game_item::EquippedComponent>().Has(weapon_id));
    ASSERT_TRUE(registry.Storage<game_item::RangedWeaponComponent>().Has(weapon_id));
    ASSERT_TRUE(registry.Storage<game_item::LoadedAmmoComponent>().Has(weapon_id));
    EXPECT_EQ(registry.Storage<game_item::LoadedAmmoComponent>().Get(weapon_id)->amount, 20);
}
