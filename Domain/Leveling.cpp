#include "Leveling.h"
#include "Components.h"
#include "Hero.h"
#include "Registry.h"
#include "Roll.h"
#include "Tables.h"
#include <algorithm>
#include <cstddef>
#include <utility>

namespace leveling {
LevelUpResult AwardXp(domain::Registry &registry, AwardXpArgs args) {
    auto *xp_comp = registry.Storage<hero::HeroExperience>().Get(args.id);
    if (xp_comp == nullptr) {
        return {};
    }
    xp_comp->xp += args.amount;

    LevelUpResult result{.level_after = xp_comp->level};
    while (xp_comp->level < hero::kMaxPlayerLevel &&
           xp_comp->xp >= hero::kExpLevelTable[xp_comp->level - 1]) {
        xp_comp->xp -= hero::kExpLevelTable[xp_comp->level - 1];
        xp_comp->level += 1;
        result.leveled_up = true;
        result.level_after = xp_comp->level;

        auto hclass_idx = std::to_underlying(xp_comp->hclass);
        auto *health = registry.Storage<component::HealthComponent>().Get(args.id);
        if (health != nullptr) {
            health->max = hero::kHpLevelTable[hclass_idx][xp_comp->level - 1];
            health->current = health->max;
        }
        auto *mana = registry.Storage<component::ManaComponent>().Get(args.id);
        if (mana != nullptr) {
            mana->max = hero::kManaLevelTable[hclass_idx][xp_comp->level - 1];
            mana->current = mana->max;
        }
    }
    return result;
}

bool TrySkillGrowth(hero::HeroSkills &skills, hero::Skill skill) {
    constexpr int kMaxSkillValue = 100;
    int die = 0;
    switch (skill) {
    case hero::Skill::HandWeapon:
        die = 50;
        break;
    case hero::Skill::Defence:
    case hero::Skill::TrapSearch:
        die = 100;
        break;
    case hero::Skill::RangedWeapon:
        die = 35;
        break;
    default:
        return false;
    }
    if (roll::Index(die) != 0) {
        return false;
    }
    auto idx = static_cast<std::size_t>(skill);
    skills.skills[idx] = std::min(kMaxSkillValue, skills.skills[idx] + 1);
    return true;
}
} // namespace leveling
