#include "Systems.h"
#include "Combat.h"
#include "Components.h"
#include "EntityStorage.h"
#include "Event.h"
#include "GameDatabase.h"
#include "GameItem.h"
#include "Hero.h"
#include "Leveling.h"
#include "Location.h"
#include "Map.h"
#include "Registry.h"
#include "RegistryQueries.h"
#include "Roll.h"
#include "Tables.h"
#include "Vector2Int.h"
#include <algorithm>
#include <array>
#include <cstddef>
#include <optional>
#include <utility>
#include <vector>

namespace systems {

void StampDamage(domain::Registry &registry, event::Result &result,
                 std::optional<entity::EntityId> attacker, entity::EntityId target, int damage,
                 event::Type event_type) {
    result.events.push_back(event::Event{
        .type = event_type, .attacker = attacker, .defender = target, .value = damage});

    auto *cur_damage = registry.Storage<component::IntentToTakeDamage>().Get(target);
    if (cur_damage == nullptr) {
        registry.Storage<component::IntentToTakeDamage>().Add(
            target, {.amount = damage, .attacker = attacker});
        return;
    }
    cur_damage->amount += damage;

    if (attacker.has_value()) {
        cur_damage->attacker = attacker;
    }
}

event::Result MeleeSystem(domain::Registry &registry) {
    event::Result result{};

    for (auto entity_id :
         registry.View<component::IntentToMelee, component::AttributesComponent>()) {
        auto &intent = registry.GetRef<component::IntentToMelee>(entity_id);
        auto &attacker_attr = registry.GetRef<component::AttributesComponent>(entity_id);
        const auto *target_attr =
            registry.Storage<component::AttributesComponent>().Get(intent.target_entity_id);
        if (target_attr == nullptr) {
            continue;
        }
        if (!combat::RollsToHit(registry, entity_id, hero::Skill::HandWeapon)) {
            result.events.push_back(event::Event{.type = event::Type::Missed,
                                                 .attacker = entity_id,
                                                 .defender = intent.target_entity_id});
            continue;
        }
        if (combat::RollsToDodge(registry, intent.target_entity_id)) {
            result.events.push_back(event::Event{.type = event::Type::Avoided,
                                                 .attacker = entity_id,
                                                 .defender = intent.target_entity_id});
            continue;
        }
        auto weapon = combat::EffectiveMelee(registry, entity_id);
        auto armor = combat::EffectiveArmor(registry, intent.target_entity_id);
        auto damage = combat::ResolveMelee(attacker_attr, weapon, *target_attr, armor);
        if (damage > 0) {
            StampDamage(registry, result, entity_id, intent.target_entity_id, damage);
        }
    }

    registry.Storage<component::IntentToMelee>().Clear();

    return result;
}

namespace {

std::optional<entity::EntityId> FindEnemyAlongRay(const domain::Registry &registry,
                                                  entity::EntityId attacker, Location origin,
                                                  Vector2Int direction, int range) {
    for (int i = 1; i <= range; i++) {
        auto check_loc = origin + direction * i;
        if (!PassableTerrainAt(registry, check_loc)) {
            break;
        }

        auto body = BlockingEntityIdAt(registry, check_loc);
        if (!body.has_value()) {
            continue;
        }
        return AreEnemies(registry, attacker, *body) ? body : std::nullopt;
    }
    return std::nullopt;
}

// Closest enemy on any cardinal ray; ties break by direction order (Up, Right, Down, Left).
std::optional<std::pair<entity::EntityId, Vector2Int>>
FindNearestEnemyAlongCardinalRays(const domain::Registry &registry, entity::EntityId attacker,
                                  Location origin, int range) {
    static const std::array<Vector2Int, 4> kDirections{Vector2Int::Up(), Vector2Int::Right(),
                                                       Vector2Int::Down(), Vector2Int::Left()};
    std::optional<std::pair<entity::EntityId, Vector2Int>> best;
    int best_distance = range + 1;

    for (auto direction : kDirections) {
        auto target = FindEnemyAlongRay(registry, attacker, origin, direction, range);
        if (!target.has_value()) {
            continue;
        }
        auto distance = origin.coord.DistanceTo(registry.GetRef<Location>(*target).coord);
        if (distance < best_distance) {
            best_distance = distance;
            best = std::make_pair(*target, direction);
        }
    }
    return best;
}
} // namespace

event::Result RangedSystem(domain::Registry &registry) {
    event::Result result{};

    for (auto entity_id : registry.View<component::IntentToShoot, Location>()) {
        auto &intent = registry.GetRef<component::IntentToShoot>(entity_id);
        auto &shooter_loc = registry.GetRef<Location>(entity_id);

        auto weapon_id = combat::EffectiveRangedWeaponId(registry, entity_id);
        if (!weapon_id.has_value()) {
            result.events.push_back(event::Event{.type = event::Type::NoWeapon});
            continue;
        }
        auto &weapon = *registry.Storage<game_item::RangedWeaponComponent>().Get(*weapon_id);
        auto *ammo = registry.Storage<game_item::LoadedAmmoComponent>().Get(*weapon_id);
        if (ammo == nullptr || ammo->amount <= 0) {
            result.events.push_back(event::Event{.type = event::Type::NoAmmo});
            continue;
        }

        component::AttributesComponent attacker_attr{};
        if (const auto *attr = registry.Storage<component::AttributesComponent>().Get(entity_id)) {
            attacker_attr = *attr;
        }

        auto target =
            FindEnemyAlongRay(registry, entity_id, shooter_loc, intent.direction, weapon.range);
        if (target.has_value()) {
            if (!combat::RollsToHit(registry, entity_id, hero::Skill::RangedWeapon)) {
                result.events.push_back(event::Event{
                    .type = event::Type::Missed, .attacker = entity_id, .defender = target});
            } else if (combat::RollsToDodge(registry, *target)) {
                result.events.push_back(event::Event{
                    .type = event::Type::Avoided, .attacker = entity_id, .defender = target});
            } else {
                const auto *target_attr =
                    registry.Storage<component::AttributesComponent>().Get(*target);
                if (target_attr != nullptr) {
                    auto armor = combat::EffectiveArmor(registry, *target);
                    auto damage = combat::ResolveRanged(attacker_attr, weapon, *target_attr, armor);
                    if (damage > 0) {
                        StampDamage(registry, result, entity_id, *target, damage);
                    }
                }
            }
        }

        ammo->amount -= 1;
    }

    registry.Storage<component::IntentToShoot>().Clear();

    return result;
}

event::Result SpellSystem(domain::Registry &registry, const database::GameDatabase &db) {
    event::Result result{};

    for (auto entity_id : registry.View<component::IntentToCastSpell, Location>()) {
        auto &intent = registry.GetRef<component::IntentToCastSpell>(entity_id);
        auto &caster_loc = registry.GetRef<Location>(entity_id);
        const auto &spell = db.GetSpell(intent.spell_id);

        if (intent.costs_mana && spell.mana_cost.has_value()) {
            auto *mana = registry.Storage<component::ManaComponent>().Get(entity_id);
            if (mana == nullptr || mana->current < spell.mana_cost->amount) {
                result.events.push_back(event::Event{.type = event::Type::NoMana});
                continue;
            }
            mana->current -= spell.mana_cost->amount;
        }

        if (spell.damage.has_value()) {
            std::optional<entity::EntityId> target;
            if (intent.direction.has_value()) {
                target = FindEnemyAlongRay(registry, entity_id, caster_loc, *intent.direction,
                                           kSpellRange);
            } else if (auto nearest = FindNearestEnemyAlongCardinalRays(registry, entity_id,
                                                                        caster_loc, kSpellRange);
                       nearest.has_value()) {
                target = nearest->first;
            }
            if (target.has_value()) {
                auto damage = combat::ResolveSpell(*spell.damage);
                StampDamage(registry, result, entity_id, *target, damage);
            }
        }

        if (spell.healing.has_value()) {
            registry.Storage<component::IntentToHeal>().Add(entity_id,
                                                            {.amount = spell.healing->amount});
        }
    }

    registry.Storage<component::IntentToCastSpell>().Clear();

    return result;
}

namespace {
void ResolveTrapEncounter(domain::Registry &registry, event::Result &result, entity::EntityId id,
                          Location loc) {

    entity::EntityId trap_id = 0;
    map::TrapComponent *trap = nullptr;
    for (auto ent_id : systems::EntitiesAt(registry, loc)) {
        if (registry.Storage<map::TrapComponent>().Has(ent_id)) {
            trap = registry.Storage<map::TrapComponent>().Get(ent_id);
            if (!trap->active) {
                return;
            }
            trap_id = ent_id;
            break;
        }
    }
    if (trap_id == 0) {
        return;
    }

    auto *skills = registry.Storage<hero::HeroSkills>().Get(id);
    const bool disarmed =
        skills != nullptr &&
        roll::SkillCheck(skills->skills[static_cast<std::size_t>(hero::Skill::TrapSearch)],
                         trap->difficulty);
    if (disarmed) {
        leveling::TrySkillGrowth(*skills, hero::Skill::TrapSearch);
        result.events.push_back(event::Event{.type = event::Type::TrapDisarmed, .defender = id});
    } else {
        auto *health = registry.Storage<component::HealthComponent>().Get(id);
        auto trap_damage = map::TrapDamage(health->max);
        // No deaths from traps
        trap_damage = std::min(trap_damage, std::max(0, health->current - 1));
        StampDamage(registry, result, std::nullopt, id, trap_damage, event::Type::TrapDamage);
    }
    trap->active = false;
}

void ResolveHealingSource(domain::Registry &registry, entity::EntityId id, Location loc) {
    entity::EntityId healing_source_id = 0;
    const map::HealingSourceComponent *healing_source = nullptr;
    for (auto ent_id : systems::EntitiesAt(registry, loc)) {
        if (registry.Storage<map::HealingSourceComponent>().Has(ent_id)) {
            healing_source = registry.Storage<map::HealingSourceComponent>().Get(ent_id);
            healing_source_id = ent_id;
            break;
        }
    }
    if (healing_source_id == 0) {
        return;
    }
    registry.Storage<component::IntentToHeal>().Add(id, {.amount = healing_source->amount});
    registry.DestroyEntity(healing_source_id);
}
} // namespace

event::Result MovementSystem(domain::Registry &registry) {
    event::Result result{};
    for (auto id : registry.View<component::IntentToMove, Location>()) {
        auto &intent = registry.GetRef<component::IntentToMove>(id);
        const auto &location = registry.GetRef<Location>(id);

        auto &map = registry.maps[location.map_level];

        bool moved = false;
        const Location new_loc = location + intent.dcoord;

        if (PassableTerrainAt(registry, new_loc) && !BlockingEntityAt(registry, new_loc)) {
            registry.PlaceEntity(id, new_loc);
            moved = true;

            if (registry.Storage<component::PlayerTag>().Has(id)) {
                auto &vision = registry.GetRef<component::VisionComponent>(id);
                map.SetAreaVisible(hero::GetVisibleArea(new_loc, vision.view_zone));
            }
            ResolveTrapEncounter(registry, result, id, new_loc);
            ResolveHealingSource(registry, id, new_loc);
        }

        if (!moved && registry.Storage<component::PlayerTag>().Has(id)) {
            result.events.push_back(event::Event{.type = event::Type::Blocked, .attacker = id});
        }
    }
    registry.Storage<component::IntentToMove>().Clear();
    return result;
}

void AISystem(domain::Registry &registry) {
    auto hero_id = PlayerId(registry);
    if (hero_id == std::nullopt) {
        return;
    }

    const auto &hero_loc = registry.GetRef<Location>(hero_id.value());

    for (auto id : registry.View<component::MonsterTag, Location, component::VisionComponent>()) {
        if (registry.Storage<component::ShopkeeperTag>().Has(id)) {
            continue;
        }
        auto &monster_loc = registry.GetRef<Location>(id);
        auto &monster_vision = registry.GetRef<component::VisionComponent>(id);

        if (monster_loc.map_level != hero_loc.map_level) {
            continue;
        }
        if (monster_loc.coord.DistanceTo(hero_loc.coord) > monster_vision.view_zone) {
            continue;
        }

        auto d_x = combat::Sgn(hero_loc.coord.x - monster_loc.coord.x);
        auto d_y = combat::Sgn(hero_loc.coord.y - monster_loc.coord.y);
        if (d_x == 0 && d_y == 0) {
            continue;
        }

        if (d_x != 0 && d_y != 0) {
            if (roll::Index(2) == 0) {
                d_y = 0;
            } else {
                d_x = 0;
            }
        }
        registry.Storage<component::IntentToMove>().Add(id, {.dcoord = {.x = d_x, .y = d_y}});
    }
}

event::Result CollisionSystem(domain::Registry &registry) {
    event::Result result{};
    for (auto id : registry.View<component::IntentToMove, Location>()) {
        auto &intent = registry.GetRef<component::IntentToMove>(id);
        auto &location = registry.GetRef<Location>(id);

        auto target_entities = EntitiesAt(registry, location + intent.dcoord);
        for (auto target_entity : target_entities) {
            if (registry.Storage<component::ShopkeeperTag>().Has(target_entity) &&
                registry.Storage<component::PlayerTag>().Has(id)) {
                registry.Storage<component::IntentToMove>().Remove(id);
                result.events.push_back(
                    {.type = event::Type::Trade, .attacker = id, .defender = target_entity});
            } else if (AreEnemies(registry, target_entity, id)) {
                registry.Storage<component::IntentToMelee>().Add(
                    id, {.target_entity_id = target_entity});
                registry.Storage<component::IntentToMove>().Remove(id);
            }
        }
    }
    return result;
}

} // namespace systems
