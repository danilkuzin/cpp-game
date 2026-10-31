#include "Session.h"
#include "EntityStorage.h"
#include "Event.h"
#include "EventFormat.h"
#include "RegistryQueries.h"
#include <string>

namespace game_states {

entity::EntityId HeroId(const Session &session) {
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    return systems::PlayerId(session.registry).value();
}

void Log(Session &session, const std::string &message) { session.runtime.log.Push(message); }

void LogEvents(Session &session, const event::Result &result) {
    session.runtime.log.PushAll(ui::FormatEventResult(result, session.registry));
}

} // namespace game_states
