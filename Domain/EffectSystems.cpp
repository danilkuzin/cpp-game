#include "EffectSystems.h"
#include "Components.h"
#include "EntityFactory.h"
#include "EntityStorage.h"
#include "Event.h"
#include "GameDatabase.h"
#include "Inventory.h"
#include "Leveling.h"
#include "Location.h"
#include "Registry.h"
#include <algorithm>
#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace systems {

namespace {
// DeathSystem destroys id before GameState ever formats the event naming it — a registry lookup
// at format time would read a gone NameComponent. Read the name here instead, while id is still
// alive, and carry the string on the event. Optional because an attacker isn't always known (a
// trap kill) or, rarely, may have died earlier in this same pass.
std::optional<std::string> NameIfAlive(const domain::Registry &registry,
                                       std::optional<entity::EntityId> id) {
    if (!id.has_value()) {
        return std::nullopt;
    }
    const auto *name = registry.Storage<component::NameComponent>().Get(*id);
    return name != nullptr ? std::optional{name->name} : std::nullopt;
}
} // namespace

void DamageSystem(domain::Registry &registry) {
    for (auto id : registry.View<component::IntentToTakeDamage, component::HealthComponent>()) {
        auto &health = registry.GetRef<component::HealthComponent>(id);
        auto &damage = registry.GetRef<component::IntentToTakeDamage>(id);

        health.current = std::max(0, health.current - damage.amount);
    }
}

event::Result HealSystem(domain::Registry &registry) {
    event::Result result{};
    for (auto id : registry.View<component::IntentToHeal, component::HealthComponent>()) {
        auto &health = registry.GetRef<component::HealthComponent>(id);
        auto &heal = registry.GetRef<component::IntentToHeal>(id);

        health.current = std::min(health.current + heal.amount, health.max);
        result.events.push_back(
            event::Event{.type = event::Type::Healed, .defender = id, .value = heal.amount});
    }
    registry.Storage<component::IntentToHeal>().Clear();
    return result;
}

event::Result DeathSystem(domain::Registry &registry, const database::GameDatabase &db) {
    event::Result result{};
    for (auto id : registry.View<component::HealthComponent>()) {
        if (registry.GetRef<component::HealthComponent>(id).current > 0) {
            continue;
        }
        if (registry.Storage<component::MonsterTag>().Has(id)) {
            std::optional<entity::EntityId> attacker;
            if (const auto *damage = registry.Storage<component::IntentToTakeDamage>().Get(id)) {
                attacker = damage->attacker;
            }
            if (attacker.has_value()) {
                leveling::AwardXp(registry, {.id = *attacker, .amount = leveling::kKillXp});
            }
            result.events.push_back(event::Event{.type = event::Type::MonsterSlained,
                                                 .attacker = attacker,
                                                 .defender = id,
                                                 .attacker_name = NameIfAlive(registry, attacker),
                                                 .defender_name = NameIfAlive(registry, id)});
        }
        if (registry.Storage<component::PlayerTag>().Has(id)) {
            result.events.push_back(event::Event{.type = event::Type::HeroDied,
                                                 .defender = id,
                                                 .defender_name = NameIfAlive(registry, id)});
        }

        for (auto item_id : game_item::OwnedItems(registry, id)) {
            game_item::Drop(registry, {.item = item_id, .actor = id});
        }
        const auto money = game_item::Money(registry, id);
        if (money > 0) {
            entity::CreateCurrency(registry, db, registry.GetRef<Location>(id), money);
        }
        registry.DestroyEntity(id);
    }
    registry.Storage<component::IntentToTakeDamage>().Clear();
    return result;
}
} // namespace systems
