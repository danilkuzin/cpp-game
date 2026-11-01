#include "RegistryQueries.h"
#include "Components.h"
#include "EntityStorage.h"
#include "Location.h"
#include "Map.h"
#include "RectInt.h"
#include "Registry.h"
#include "Roll.h"
#include "Vector2Int.h"
#include <array>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

namespace systems {
namespace {
constexpr int kMaxPlacementSampleAttempts = 10;
} // namespace

std::vector<entity::EntityId> EntitiesAt(const domain::Registry &registry, Location loc) {
    std::vector<entity::EntityId> cur_entities;
    for (const auto &[entity_id, loc_iter] : registry.Storage<Location>()) {
        if (loc_iter == loc) {
            cur_entities.push_back(entity_id);
        }
    }
    return cur_entities;
}

std::optional<entity::EntityId> BlockingEntityIdAt(const domain::Registry &registry, Location loc) {
    for (auto id : EntitiesAt(registry, loc)) {
        if (registry.Storage<component::MonsterTag>().Has(id) ||
            registry.Storage<component::PlayerTag>().Has(id)) {
            return id;
        }
    }
    return std::nullopt;
}

bool BlockingEntityAt(const domain::Registry &registry, Location loc) {
    return BlockingEntityIdAt(registry, loc).has_value();
}

bool TileIsEmpty(const domain::Registry &registry, Location loc) {
    return EntitiesAt(registry, loc).empty();
}

bool PassableTerrainAt(const domain::Registry &registry, Location loc) {
    const auto &map = registry.maps[loc.map_level];
    if (!map.Contains(loc.coord)) {
        return false;
    }
    const auto &cell = map.At(loc.coord);

    return registry.terrain_registry.IsPassable(cell.terrain_id);
}

bool IsPassableAndUnoccupied(const domain::Registry &registry, const Location loc) {
    return PassableTerrainAt(registry, loc) && TileIsEmpty(registry, loc);
}

std::optional<entity::EntityId> ItemIdAt(const domain::Registry &registry, Location loc) {
    for (auto id : EntitiesAt(registry, loc)) {
        if (registry.Storage<component::ItemTag>().Has(id)) {
            return id;
        }
    }
    return std::nullopt;
}

bool AreEnemies(const domain::Registry &registry, entity::EntityId a, entity::EntityId b) {
    if (registry.Storage<component::ShopkeeperTag>().Has(a) ||
        registry.Storage<component::ShopkeeperTag>().Has(b)) {
        return false;
    }

    auto a_is_monster = registry.Storage<component::MonsterTag>().Has(a);
    auto b_is_monster = registry.Storage<component::MonsterTag>().Has(b);
    auto a_is_player = registry.Storage<component::PlayerTag>().Has(a);
    auto b_is_player = registry.Storage<component::PlayerTag>().Has(b);

    return ((a_is_monster && b_is_player) || (b_is_monster && a_is_player));
}

std::optional<entity::EntityId> PlayerId(const domain::Registry &registry) {
    std::vector<entity::EntityId> hero_ids;
    for (const auto &kv : registry.Storage<component::PlayerTag>()) {
        hero_ids.push_back(kv.first);
    }
    if (hero_ids.size() > 1) {
        throw std::runtime_error("Max 1 hero is allowed");
    }
    if (hero_ids.empty()) {
        return std::nullopt;
    }
    return hero_ids.front();
}

const map::Map *PlayerMap(const domain::Registry &registry) {
    auto hero_id = PlayerId(registry);
    if (!hero_id.has_value()) {
        return nullptr;
    }
    return &registry.maps[registry.GetRef<Location>(*hero_id).map_level];
}

std::vector<Vector2Int> SpellRayTiles(const domain::Registry &registry, entity::EntityId caster,
                                      Vector2Int direction) {
    const auto &caster_loc = registry.GetRef<Location>(caster);
    std::vector<Vector2Int> tiles;
    for (int i = 1; i <= kSpellRange; i++) {
        auto check_loc = caster_loc + direction * i;
        if (!PassableTerrainAt(registry, check_loc)) {
            break;
        }

        tiles.push_back(check_loc.coord);
        auto body = BlockingEntityIdAt(registry, check_loc);
        if (body.has_value()) {
            break; // the body's own tile is included: a spell that lands here still hits it
        }
    }
    return tiles;
}

std::optional<Vector2Int> UnoccupiedPointAtArea(const domain::Registry &registry, int map_level,
                                                RectInt area) {
    if (area.width <= 0 || area.height <= 0) {
        return std::nullopt;
    }

    for (int attempt = 0; attempt < kMaxPlacementSampleAttempts; attempt++) {
        const Vector2Int candidate = {.x = area.x + roll::Index(area.width),
                                      .y = area.y + roll::Index(area.height)};
        const Location loc = {.coord = candidate, .map_level = map_level};
        if (IsPassableAndUnoccupied(registry, loc)) {
            return candidate;
        }
    }

    for (int x = area.x; x < area.x + area.width; x++) {
        for (int y = area.y; y < area.y + area.height; y++) {
            const Vector2Int candidate = {.x = x, .y = y};
            const Location loc = {.coord = candidate, .map_level = map_level};
            if (IsPassableAndUnoccupied(registry, loc)) {
                return candidate;
            }
        }
    }
    return std::nullopt;
}

std::optional<Vector2Int> UnoccupiedPoint(const domain::Registry &registry, int map_level) {
    return UnoccupiedPointAtArea(registry, map_level, registry.maps[map_level].Bounds());
}
} // namespace systems
