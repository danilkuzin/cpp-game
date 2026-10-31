#pragma once

#include "Camera.h"
#include "Config.h"
#include "EntityStorage.h"
#include "Event.h"
#include "GameDatabase.h"
#include "MessageLog.h"
#include "Registry.h"
#include <string>
#include <vector>

namespace game_states {

struct RuntimeState {
    ui::Camera camera;
    ui::MessageLog log;
};

struct GameStats {
    std::string hero_name;
    int level = 0;
    int monsters_slain = 0;
    int money = 0;
    std::vector<std::string> equipment;
};

struct Session {
    domain::Config config;
    domain::Registry registry;
    RuntimeState runtime;
    database::GameDatabase db;
    GameStats stats;
};

[[nodiscard]] entity::EntityId HeroId(const Session &session);

void Log(Session &session, const std::string &message);

void LogEvents(Session &session, const event::Result &result);

} // namespace game_states
