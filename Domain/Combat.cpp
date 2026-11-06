#include "Combat.h"
#include "Components.h"
#include "EntityStorage.h"
#include "GameItem.h"
#include "Hero.h"
#include "Inventory.h"
#include "Leveling.h"
#include "Registry.h"
#include "Roll.h"
#include "Spell.h"
#include "Tables.h"
#include <algorithm>
#include <cstddef>
#include <optional>

namespace combat {

game_item::MeleeWeaponComponent EffectiveMelee(const domain::Registry &registry,
                                               entity::EntityId id) {
    for (auto item_id : game_item::EquippedItems(registry, id)) {
        const auto *weapon = registry.Storage<game_item::MeleeWeaponComponent>().Get(item_id);
        if (weapon != nullptr) {
            return *weapon;
        }
    }
    int str = 0;
    if (const auto *attr = registry.Storage<component::AttributesComponent>().Get(id)) {
        str = attr->chars[hero::kChrStr];
    }
    return {.damage = {.count = 1, .sides = kUnarmedBaseSides + str}};
}

int EffectiveArmor(const domain::Registry &registry, entity::EntityId id) {
    int total = 0;
    for (auto item_id : game_item::EquippedItems(registry, id)) {
        const auto *armor = registry.Storage<game_item::ArmorComponent>().Get(item_id);
        if (armor != nullptr) {
            total += armor->defence;
        }
    }
    return total;
}

int ResolveMelee(const component::AttributesComponent &att_attr,
                 const game_item::MeleeWeaponComponent &weapon,
                 const component::AttributesComponent &def_attr, int def_armor) {
    int damage = roll::Dice(weapon.damage);
    damage += att_attr.chars[hero::kChrStr] / 2;

    const int mitigation = (def_attr.chars[hero::kChrStr] / 2) + def_armor;
    damage -= mitigation;

    damage = std::max(kMinLandedDamage, damage);
    return damage;
}

std::optional<entity::EntityId> EffectiveRangedWeaponId(const domain::Registry &registry,
                                                        entity::EntityId id) {
    for (auto item_id : game_item::EquippedItems(registry, id)) {
        if (registry.Storage<game_item::RangedWeaponComponent>().Has(item_id)) {
            return item_id;
        }
    }
    return std::nullopt;
}

std::optional<game_item::RangedWeaponComponent> EffectiveRanged(const domain::Registry &registry,
                                                                entity::EntityId id) {
    auto weapon_id = EffectiveRangedWeaponId(registry, id);
    if (!weapon_id.has_value()) {
        return std::nullopt;
    }
    return *registry.Storage<game_item::RangedWeaponComponent>().Get(*weapon_id);
}

int ResolveRanged(const component::AttributesComponent &att_attr,
                  const game_item::RangedWeaponComponent &weapon,
                  const component::AttributesComponent &def_attr, int def_armor) {
    int damage = roll::Dice(weapon.damage);
    damage += att_attr.chars[hero::kChrDex] / 2;

    const int mitigation = (def_attr.chars[hero::kChrStr] / 2) + def_armor;
    damage -= mitigation;

    damage = std::max(kMinLandedDamage, damage);
    return damage;
}

int ResolveSpell(const spell::DamageComponent &damage) { return damage.amount; }

bool RollsToHit(domain::Registry &registry, entity::EntityId attacker, hero::Skill weapon_skill) {
    auto *skills = registry.Storage<hero::HeroSkills>().Get(attacker);
    if (skills == nullptr) {
        return true;
    }
    const bool hit = roll::SkillCheck(skills->skills[static_cast<std::size_t>(weapon_skill)], 0);
    if (hit) {
        leveling::TrySkillGrowth(*skills, weapon_skill);
    }
    return hit;
}

bool RollsToDodge(domain::Registry &registry, entity::EntityId defender) {
    auto *skills = registry.Storage<hero::HeroSkills>().Get(defender);
    if (skills == nullptr) {
        return false;
    }
    const bool dodged =
        roll::SkillCheck(skills->skills[static_cast<std::size_t>(hero::Skill::Defence)], 0);
    if (dodged) {
        leveling::TrySkillGrowth(*skills, hero::Skill::Defence);
    }
    return dodged;
}
} // namespace combat
