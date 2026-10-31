#pragma once
#include "EntityStorage.h"
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace event {
enum class Type : std::uint8_t {
    Missed,
    NoWeapon,
    NoAmmo,
    GotDamage,
    MonsterSlained,
    TrapDamage,
    TrapDisarmed,
    HeroDied,
    Healed,
    NoItem,
    ItemPickedUp,
    ItemDropped,
    ItemUsed,
    FullInventory,
    NoMana,
    Avoided,
    Trade,
    Blocked
};

struct Event {
    Type type{};
    std::optional<entity::EntityId> attacker = std::nullopt;
    std::optional<entity::EntityId> defender = std::nullopt;
    int value = 0;
    // Set whenever the producer may destroy attacker/defender's own entity before the event is
    // formatted.
    std::optional<std::string> attacker_name = std::nullopt;
    std::optional<std::string> defender_name = std::nullopt;
};

struct Result {
    std::vector<Event> events;
};
} // namespace event
