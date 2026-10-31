#pragma once
#include "EntityStorage.h"
#include "Hero.h"
#include "Spell.h"
#include "Vector2Int.h"
#include <array>
#include <optional>
#include <string>

namespace component {
struct NameComponent {
    std::string name;
};

struct RenderComponent {
    std::string tile;
    int z_index = 0;
};

struct HealthComponent {
    int current = 0;
    int max = 0;
};

struct ManaComponent {
    int current = 0;
    int max = 0;
};

struct PurseComponent {
    int money = 0;
};

struct VisionComponent {
    int view_zone = 0;
};

struct AttributesComponent {
    std::array<int, hero::kMaxChars> chars{};
};

struct PlayerTag {};

struct MonsterTag {};

struct ItemTag {};

struct ShopkeeperTag {};

struct IntentToMove {
    Vector2Int dcoord;
};

struct IntentToMelee {
    entity::EntityId target_entity_id = 0;
};

struct IntentToShoot {
    Vector2Int direction;
};

struct IntentToCastSpell {
    spell::SpellId spell_id;
    bool costs_mana = false;
    std::optional<Vector2Int> direction;
};

struct IntentToTakeDamage {
    int amount = 0;
    std::optional<entity::EntityId> attacker = std::nullopt;
};

struct IntentToHeal {
    int amount = 0;
};
} // namespace component
