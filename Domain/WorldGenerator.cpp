#include "WorldGenerator.h"
#include "Components.h"
#include "Config.h"
#include "EntityFactory.h"
#include "EntityStorage.h"
#include "GameDatabase.h"
#include "GameItem.h"
#include "Hero.h"
#include "Location.h"
#include "Map.h"
#include "Monster.h"
#include "Registry.h"
#include "RegistryQueries.h"
#include "Roll.h"
#include "Terrain.h"
#include <algorithm>
#include <cstddef>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace domain {
namespace {
constexpr int kHealingSourceAmount = 10;
constexpr int kGroundChancePercent = 35;
constexpr int kEvenOddsPercent = 50;

constexpr map::TerrainId kGrassTerrainId = 0;
constexpr map::TerrainId kGroundTerrainId = 1;
constexpr map::TerrainId kTreeTerrainId = 4;
constexpr map::TerrainId kStoneTerrainId = 5;
constexpr int kObjectChancePercent = 1;
constexpr int kItemChancePercent = 1;

std::string NoFreeTileMessage(std::size_t level, const std::string &what) {
    return "WorldGenerator::GenerateMonsters: no free tile on level " + std::to_string(level) +
           " for " + what;
}
} // namespace

void WorldGenerator::GenerateMaps(Registry &registry, const MapConfig &map_config) {
    for (int map_level = 0; map_level < map_config.max_level; map_level++) {
        registry.maps.push_back(GenerateMap(map_config));
    }
}

entity::EntityId WorldGenerator::GenerateHero(Registry &registry,
                                              entity::HeroGenerationParameters &hparams,
                                              int map_level, database::GameDatabase &db) {
    auto &map = registry.maps[map_level];
    auto coord = systems::UnoccupiedPointAtArea(registry, map_level, map.GetStartingArea());
    if (!coord.has_value()) {
        throw std::runtime_error("WorldGenerator::GenerateHero: no free tile in the starting area");
    }
    const Location hero_loc = {.coord = *coord, .map_level = map_level};
    auto hero_id = entity::CreateHero(registry, hparams, hero_loc);
    entity::GiveStartingKit(registry, db, hero_id, hparams.hclass);

    map.SetAreaVisible(hero::GetVisibleArea(
        hero_loc, registry.GetRef<component::VisionComponent>(hero_id).view_zone));

    return hero_id;
};

map::Map WorldGenerator::GenerateMap(const MapConfig &map_config) {
    map::Map map(map_config.width, map_config.height,
                 map::Cell{.terrain_id = kGrassTerrainId, .explored = false});
    for (int x = 0; x < map_config.width; x++) {
        for (int y = 0; y < map_config.height; y++) {
            map::TerrainId terrain = kGrassTerrainId;

            if (x <= map_config.border_size || x >= map_config.width - map_config.border_size ||
                y <= map_config.border_size || y >= map_config.height - map_config.border_size) {
                terrain = kStoneTerrainId;
            } else {
                if (roll::Chance(kGroundChancePercent)) {
                    terrain = kGroundTerrainId;
                } else if (roll::Chance(kEvenOddsPercent)) {
                    terrain = kGrassTerrainId;
                } else {
                    terrain = kTreeTerrainId;
                }
            }

            map.At({.x = x, .y = y}).terrain_id = terrain;
        }
    }
    return map;
}

void WorldGenerator::GenerateTileFeatures(Registry &registry) {
    for (std::size_t level = 0; level < registry.maps.size(); level++) {
        const auto map_level = static_cast<int>(level);
        const auto &map = registry.maps[level];
        for (int x = 0; x < map.Width(); x++) {
            for (int y = 0; y < map.Height(); y++) {
                const Location loc = {.coord = {.x = x, .y = y}, .map_level = map_level};
                if (!registry.terrain_registry.IsPassable(map.At(loc.coord).terrain_id)) {
                    continue;
                }
                if (!roll::Chance(kObjectChancePercent)) {
                    continue;
                }
                if (roll::Chance(kEvenOddsPercent)) {
                    int trap_difficulty =
                        15 + (map_level * 10) + roll::Dice({.count = 1, .sides = 15});
                    trap_difficulty = std::min(trap_difficulty, 95);
                    entity::CreateTrap(registry, trap_difficulty, loc);
                } else {
                    entity::CreateHealingSource(registry, kHealingSourceAmount, loc);
                }
            }
        }
    }
}

void WorldGenerator::GenerateItems(Registry &registry, database::GameDatabase &db) {
    std::vector<game_item::ItemTemplate> cur_prefabs;
    for (const auto &kv : db.GetItems()) {
        cur_prefabs.push_back(kv.second);
    }
    if (cur_prefabs.empty()) {
        return;
    }

    for (std::size_t level = 0; level < registry.maps.size(); level++) {
        const auto map_level = static_cast<int>(level);
        const auto &map = registry.maps[level];
        for (int x = 0; x < map.Width(); x++) {
            for (int y = 0; y < map.Height(); y++) {
                const Location loc = {.coord = {.x = x, .y = y}, .map_level = map_level};
                if (!registry.terrain_registry.IsPassable(map.At(loc.coord).terrain_id) ||
                    !systems::TileIsEmpty(registry, loc)) {
                    continue;
                }
                if (roll::Chance(kItemChancePercent)) {
                    auto item_type = roll::Index(static_cast<int>(cur_prefabs.size()));
                    entity::CreateItem(registry, cur_prefabs[item_type], loc, std::nullopt);
                }
            }
        }
    }
}

void WorldGenerator::GenerateMonsters(Registry &registry, database::GameDatabase &db,
                                      int num_monsters) {
    // One guaranteed shopkeeper per level
    std::optional<monster::MonsterPrefab> shopkeeper_prefab;
    for (const auto &kv : db.GetMonsters()) {
        if (kv.second.shopkeeper) {
            shopkeeper_prefab = kv.second;
            break;
        }
    }

    for (std::size_t level = 0; level < registry.maps.size(); level++) {
        std::vector<monster::MonsterPrefab> cur_prefabs;
        for (const auto &kv : db.GetMonsters() | std::views::filter([level](const auto &kv) {
                                  return static_cast<std::size_t>(kv.second.map_level) == level &&
                                         !kv.second.shopkeeper;
                              })) {
            cur_prefabs.push_back(kv.second);
        }

        if (!cur_prefabs.empty()) {
            for (auto m = 0; m < num_monsters; m++) {
                auto monster_type = roll::Index(static_cast<int>(cur_prefabs.size()));
                auto coord = systems::UnoccupiedPoint(registry, static_cast<int>(level));
                if (!coord.has_value()) {
                    const std::string what =
                        "monster " + std::to_string(m + 1) + " of " + std::to_string(num_monsters);
                    throw std::runtime_error(NoFreeTileMessage(level, what));
                }

                const Location proposed_loc = {.coord = *coord,
                                               .map_level = static_cast<int>(level)};
                auto entity_id =
                    entity::CreateMonster(registry, cur_prefabs[monster_type], proposed_loc);
                entity::GiveMonsterKit(registry, db, entity_id);
            }
        }

        if (shopkeeper_prefab.has_value()) {
            auto shop_coord = systems::UnoccupiedPoint(registry, static_cast<int>(level));
            if (!shop_coord.has_value()) {
                throw std::runtime_error(NoFreeTileMessage(level, "the guaranteed shopkeeper"));
            }
            const Location shop_loc = {.coord = *shop_coord, .map_level = static_cast<int>(level)};
            auto shopkeeper_id = entity::CreateMonster(registry, *shopkeeper_prefab, shop_loc);
            entity::GiveShopkeeperStock(registry, db, shopkeeper_id);
        }
    }
}

} // namespace domain
