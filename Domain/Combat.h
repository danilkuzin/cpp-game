#pragma once
#include "Components.h"
#include "EntityStorage.h"
#include "GameItem.h"
#include "Registry.h"
#include "Spell.h"
#include "Tables.h"
#include <optional>

namespace combat {
inline constexpr int kUnarmedBaseSides = 2;
inline constexpr int kMinLandedDamage = 1;
inline constexpr game_item::MeleeWeaponComponent kUnarmedMelee{
    .damage = {.count = 1, .sides = kUnarmedBaseSides}};

game_item::MeleeWeaponComponent EffectiveMelee(const domain::Registry &registry,
                                               entity::EntityId id);
int EffectiveArmor(const domain::Registry &registry, entity::EntityId id);

int ResolveMelee(const component::AttributesComponent &att_attr,
                 const game_item::MeleeWeaponComponent &weapon,
                 const component::AttributesComponent &def_attr, int def_armor);

bool RollsToHit(domain::Registry &registry, entity::EntityId attacker, hero::Skill weapon_skill);
bool RollsToDodge(domain::Registry &registry, entity::EntityId defender);

std::optional<entity::EntityId> EffectiveRangedWeaponId(const domain::Registry &registry,
                                                        entity::EntityId id);
std::optional<game_item::RangedWeaponComponent> EffectiveRanged(const domain::Registry &registry,
                                                                entity::EntityId id);

int ResolveRanged(const component::AttributesComponent &att_attr,
                  const game_item::RangedWeaponComponent &weapon,
                  const component::AttributesComponent &def_attr, int def_armor);

int ResolveSpell(const spell::DamageComponent &damage);
} // namespace combat
