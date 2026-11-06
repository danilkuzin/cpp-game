#include "Components.h"
#include "EntityStorage.h"
#include "Hero.h"
#include "Leveling.h"
#include "Registry.h"
#include "Tables.h"
#include <cstddef>
#include <gtest/gtest.h>
#include <utility>

namespace {
// NOLINTBEGIN(bugprone-easily-swappable-parameters)
entity::EntityId MakeHero(domain::Registry &registry, int level, int xp, hero::HClass hclass,
                          int hp_current, int hp_max, int mana_current, int mana_max) {
    // NOLINTEND(bugprone-easily-swappable-parameters)
    auto id = registry.entities.CreateEntity();
    registry.Storage<hero::HeroExperience>().Add(id, {.xp = xp, .level = level, .hclass = hclass});
    registry.Storage<component::HealthComponent>().Add(id, {.current = hp_current, .max = hp_max});
    registry.Storage<component::ManaComponent>().Add(id,
                                                     {.current = mana_current, .max = mana_max});
    return id;
}
} // namespace

TEST(Leveling, AwardXpBelowThresholdJustAccumulates) {
    domain::Registry registry;
    auto id = MakeHero(registry, 1, 0, hero::HClass::Warrior, 10, 10, 0, 0);

    auto result = leveling::AwardXp(registry, {.id = id, .amount = 5});

    EXPECT_FALSE(result.leveled_up);
    EXPECT_EQ(result.level_after, 1);
    EXPECT_EQ(registry.Storage<hero::HeroExperience>().Get(id)->xp, 5);
    EXPECT_EQ(registry.Storage<hero::HeroExperience>().Get(id)->level, 1);
    EXPECT_EQ(registry.Storage<component::HealthComponent>().Get(id)->max, 10);
}

TEST(Leveling, AwardXpCrossingOneThresholdLevelsUpAndFullyHeals) {
    domain::Registry registry;
    auto id = MakeHero(registry, 1, 0, hero::HClass::Mage, 1, 4, 1, 8);

    auto result = leveling::AwardXp(registry, {.id = id, .amount = 15});

    EXPECT_TRUE(result.leveled_up);
    EXPECT_EQ(result.level_after, 2);
    auto *xp_comp = registry.Storage<hero::HeroExperience>().Get(id);
    EXPECT_EQ(xp_comp->level, 2);
    EXPECT_EQ(xp_comp->xp, 5);
    auto *health = registry.Storage<component::HealthComponent>().Get(id);
    EXPECT_EQ(health->max, 7);
    EXPECT_EQ(health->current, 7);
    auto *mana = registry.Storage<component::ManaComponent>().Get(id);
    EXPECT_EQ(mana->max, 12);
    EXPECT_EQ(mana->current, 12);
}

TEST(Leveling, HugeAwardCascadesMultipleLevelsAndStopsAtCap) {
    domain::Registry registry;
    auto id = MakeHero(registry, 1, 0, hero::HClass::Warrior, 10, 10, 0, 0);

    auto result = leveling::AwardXp(registry, {.id = id, .amount = 2000});

    EXPECT_TRUE(result.leveled_up);
    EXPECT_EQ(result.level_after, hero::kMaxPlayerLevel);
    EXPECT_EQ(registry.Storage<hero::HeroExperience>().Get(id)->level, hero::kMaxPlayerLevel);
    EXPECT_EQ(
        registry.Storage<component::HealthComponent>().Get(id)->max,
        hero::kHpLevelTable[std::to_underlying(hero::HClass::Warrior)][hero::kMaxPlayerLevel - 1]);
}

TEST(Leveling, LevelNeverExceedskMaxPlayerLevelEvenWithMassiveExcessXp) {
    domain::Registry registry;
    auto id = MakeHero(registry, 1, 0, hero::HClass::Warrior, 10, 10, 0, 0);

    leveling::AwardXp(registry, {.id = id, .amount = 1'000'000});

    EXPECT_EQ(registry.Storage<hero::HeroExperience>().Get(id)->level, hero::kMaxPlayerLevel);
}

TEST(Leveling, AwardXpOnEntityWithoutHeroExperienceIsSafeNoOp) {
    domain::Registry registry;
    auto id = registry.entities.CreateEntity();

    auto result = leveling::AwardXp(registry, {.id = id, .amount = 100});

    EXPECT_FALSE(result.leveled_up);
    EXPECT_EQ(result.level_after, 0);
}

TEST(Leveling, TrySkillGrowthNeverExceedsMax) {
    hero::HeroSkills skills{.skills = {100, 100, 100, 100, 100}};
    for (int i = 0; i < 500; i++) {
        leveling::TrySkillGrowth(skills, hero::Skill::HandWeapon);
    }
    EXPECT_EQ(skills.skills[static_cast<std::size_t>(hero::Skill::HandWeapon)], 100);
}

// Statistical
TEST(Leveling, TrySkillGrowthEventuallyGrowsTheRarestSkill) {
    hero::HeroSkills skills{.skills = {0, 0, 0, 0, 0}};
    bool grew = false;
    for (int i = 0; i < 2000; i++) {
        grew = leveling::TrySkillGrowth(skills, hero::Skill::Defence) || grew;
    }
    EXPECT_TRUE(grew);
    EXPECT_GE(skills.skills[static_cast<std::size_t>(hero::Skill::Defence)], 1);
}

TEST(Leveling, TrySkillGrowthHasNoConsumerForSpellcastingYet) {
    hero::HeroSkills skills{.skills = {0, 0, 0, 0, 0}};
    for (int i = 0; i < 500; i++) {
        EXPECT_FALSE(leveling::TrySkillGrowth(skills, hero::Skill::Spellcasting));
    }
    EXPECT_EQ(skills.skills[static_cast<std::size_t>(hero::Skill::Spellcasting)], 0);
}
