#pragma once

#include "EntityStorage.h"
#include "Location.h"
#include "Map.h"
#include "RectInt.h"
#include "Registry.h"
#include "Vector2Int.h"
#include <optional>
#include <vector>

namespace systems {
constexpr int kSpellRange = 5;

std::vector<entity::EntityId> EntitiesAt(const domain::Registry &registry, Location loc);
// A tile holds at most one creature, items don't block
std::optional<entity::EntityId> BlockingEntityIdAt(const domain::Registry &registry, Location loc);
bool BlockingEntityAt(const domain::Registry &registry, Location loc);
bool TileIsEmpty(const domain::Registry &registry, Location loc);
bool PassableTerrainAt(const domain::Registry &registry, Location loc);
bool IsPassableAndUnoccupied(const domain::Registry &registry, Location loc);
// Order of pick up is not specified
std::optional<entity::EntityId> ItemIdAt(const domain::Registry &registry, Location loc);
bool AreEnemies(const domain::Registry &registry, entity::EntityId a, entity::EntityId b);
std::optional<entity::EntityId> PlayerId(const domain::Registry &registry);
const map::Map *PlayerMap(const domain::Registry &registry);

std::optional<Vector2Int> UnoccupiedPointAtArea(const domain::Registry &registry, int map_level,
                                                RectInt area);
std::optional<Vector2Int> UnoccupiedPoint(const domain::Registry &registry, int map_level);

std::vector<Vector2Int> SpellRayTiles(const domain::Registry &registry, entity::EntityId caster,
                                      Vector2Int direction);
} // namespace systems
