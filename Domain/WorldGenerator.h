#pragma once

#include "Config.h"
#include "EntityFactory.h"
#include "EntityStorage.h"
#include "GameDatabase.h"
#include "Map.h"
#include "Registry.h"

namespace domain {

// The order is important:
//
//   1. GenerateMaps
//   2. GenerateTileFeatures
//   3. GenerateItems
//   4. GenerateMonsters
//   5. GenerateHero
class WorldGenerator {
public:
    static void GenerateMaps(Registry &registry, const MapConfig &map_config);
    static void GenerateTileFeatures(Registry &registry);
    static void GenerateItems(Registry &registry, database::GameDatabase &db);
    static void GenerateMonsters(Registry &registry, database::GameDatabase &db, int num_monsters);
    static entity::EntityId GenerateHero(Registry &registry,
                                         entity::HeroGenerationParameters &hparams, int map_level,
                                         database::GameDatabase &db);

private:
    static map::Map GenerateMap(const MapConfig &map_config);
};
} // namespace domain
