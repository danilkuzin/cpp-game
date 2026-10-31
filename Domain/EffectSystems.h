#pragma once

#include "Event.h"
#include "GameDatabase.h"
#include "Registry.h"

namespace systems {
void DamageSystem(domain::Registry &registry);
event::Result HealSystem(domain::Registry &registry);
event::Result DeathSystem(domain::Registry &registry, const database::GameDatabase &db);

} // namespace systems
