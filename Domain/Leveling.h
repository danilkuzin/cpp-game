#pragma once
#include "EntityStorage.h"
#include "Hero.h"
#include "Registry.h"
#include "Tables.h"

namespace leveling {
constexpr int kKillXp = 5;

struct LevelUpResult {
    bool leveled_up = false;
    int level_after = 0;
};

struct AwardXpArgs {
    entity::EntityId id = 0;
    int amount = 0;
};

LevelUpResult AwardXp(domain::Registry &registry, AwardXpArgs args);

bool TrySkillGrowth(hero::HeroSkills &skills, hero::Skill skill);
} // namespace leveling
