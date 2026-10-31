#pragma once

#include <optional>
#include <string>

namespace spell {

using SpellId = std::string;

struct ManaCostComponent {
    int amount = 0;
};

struct DamageComponent {
    int amount = 0;
};

struct HealingComponent {
    int amount = 0;
};

struct SpellData {
    SpellId id;
    std::string name;
    std::optional<ManaCostComponent> mana_cost;
    std::optional<DamageComponent> damage;
    std::optional<HealingComponent> healing;
};

} // namespace spell
