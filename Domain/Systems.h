#pragma once

#include "EntityStorage.h"
#include "Event.h"
#include "GameDatabase.h"
#include "Registry.h"
#include <optional>

namespace systems {

void StampDamage(domain::Registry &registry, event::Result &result,
                 std::optional<entity::EntityId> attacker, entity::EntityId target, int damage,
                 event::Type event_type = event::Type::GotDamage);

event::Result CollisionSystem(domain::Registry &registry);
event::Result MeleeSystem(domain::Registry &registry);
event::Result RangedSystem(domain::Registry &registry);
event::Result SpellSystem(domain::Registry &registry, const database::GameDatabase &db);
event::Result MovementSystem(domain::Registry &registry);

void AISystem(domain::Registry &registry);

} // namespace systems
