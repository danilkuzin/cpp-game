#pragma once
#include "InputEvent.h"
#include <cstdint>
#include <optional>

namespace ui {
enum class InputContext : std::uint8_t { Gameplay, Menu, TextEntry };

enum class Action : std::uint8_t {
    MoveNorth,
    MoveSouth,
    MoveWest,
    MoveEast,
    ShootNorth,
    ShootSouth,
    ShootWest,
    ShootEast,
    CastSpell1,
    CastSpell2,
    PickUp,
    OpenInventory,
    OpenHeroInfo,
    PrevOption,
    NextOption,
    Confirm,
    Cancel,
    DeleteChar,
};

// nullopt = this key means nothing in this context, and the state never hears about it.
std::optional<Action> Resolve(InputContext context, Key key);
} // namespace ui
