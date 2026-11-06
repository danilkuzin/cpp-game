#include "Components.h"
#include "Config.h"
#include "EntityStorage.h"
#include "GameDatabase.h"
#include "GameItem.h"
#include "Location.h"
#include "Map.h"
#include "Registry.h"
#include "Roll.h"
#include "Support/MakeRegistry.h"
#include "Support/TestData.h"
#include "WorldGenerator.h"
#include <algorithm>
#include <cstddef>
#include <gtest/gtest.h>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
domain::Config MakeConfig() {
    return domain::Config{
        .map_config = {.width = 12, .height = 12, .max_level = 2, .border_size = 2},
        .num_monsters = 3,
        .seed = std::nullopt};
}

void GenerateWorldForTest(domain::Registry &registry, domain::Config &config,
                          database::GameDatabase &db) {
    domain::WorldGenerator::GenerateMaps(registry, config.map_config);
    domain::WorldGenerator::GenerateTileFeatures(registry);
    domain::WorldGenerator::GenerateItems(registry, db);
    domain::WorldGenerator::GenerateMonsters(registry, db, config.num_monsters);
}

database::GameDatabase MakeDatabase() {
    database::GameDatabase db;
    db.AddMonster({.id = "rat",
                   .name = "Rat",
                   .map_level = 0,
                   .view_zone = 4,
                   .max_hp = 3,
                   .chars = {1, 1, 0, 0, 0},
                   .tile = "../rat.bmp"});
    db.AddMonster({.id = "troll",
                   .name = "Troll",
                   .map_level = 1,
                   .view_zone = 4,
                   .max_hp = 9,
                   .chars = {3, 1, 0, 0, 0},
                   .tile = "../troll.bmp"});
    auto axe = MakeItemTemplate("axe", "Axe", 2, "../axe.bmp");
    axe.melee = game_item::MeleeWeaponComponent{.damage = {.count = 1, .sides = 6}};
    axe.value = game_item::ValueComponent{.base_price = 10};
    db.AddItem(axe);
    auto copper = MakeItemTemplate("copper", "Copper", 0, "../copper.bmp");
    copper.currency = game_item::CurrencyComponent{.amount = 0};
    db.AddItem(copper);
    return db;
}

std::string EntityLabel(const domain::Registry &registry, entity::EntityId id) {
    if (registry.Storage<map::TrapComponent>().Has(id)) {
        return "trap";
    }
    if (registry.Storage<map::HealingSourceComponent>().Has(id)) {
        return "healing";
    }
    return registry.GetRef<component::NameComponent>(id).name;
}

std::string Snapshot(unsigned int seed) {
    domain::Registry registry;
    auto config = MakeConfig();
    auto db = MakeDatabase();

    roll::Seed(seed);
    GenerateWorldForTest(registry, config, db);

    std::string snapshot;
    for (const auto &map : registry.maps) {
        for (int x = 0; x < map.Width(); x++) {
            for (int y = 0; y < map.Height(); y++) {
                snapshot += std::to_string(map.At({.x = x, .y = y}).terrain_id);
            }
        }
    }

    std::vector<std::string> placements;
    for (const auto &[id, loc] : registry.Storage<Location>()) {
        placements.push_back(EntityLabel(registry, id) + "@" + std::to_string(loc.coord.x) + "," +
                             std::to_string(loc.coord.y) + "," + std::to_string(loc.map_level));
    }
    std::ranges::sort(placements);
    for (const auto &placement : placements) {
        snapshot += placement;
    }
    return snapshot;
}
} // namespace

TEST(WorldGenerator, GenerateMapsBuildsTerrainAndSpawnsNothing) {
    domain::Registry registry;
    auto config = MakeConfig();

    roll::Seed(1U);
    domain::WorldGenerator::GenerateMaps(registry, config.map_config);

    ASSERT_EQ(registry.maps.size(), static_cast<std::size_t>(config.map_config.max_level));
    for (const auto &map : registry.maps) {
        EXPECT_EQ(map.Width(), config.map_config.width);
        EXPECT_EQ(map.Height(), config.map_config.height);
    }
    EXPECT_TRUE(registry.Storage<Location>().Empty());
}

TEST(WorldGenerator, TileFeaturesAndItemsLandOnPassableTerrainOfTheGeneratedMaps) {
    domain::Registry registry;
    auto config = MakeConfig();
    auto db = MakeDatabase();

    roll::Seed(20260819U);
    domain::WorldGenerator::GenerateMaps(registry, config.map_config);
    domain::WorldGenerator::GenerateTileFeatures(registry);
    domain::WorldGenerator::GenerateItems(registry, db);

    for (const auto &[id, loc] : registry.Storage<Location>()) {
        (void)id;
        const auto &map = registry.maps[loc.map_level];
        ASSERT_TRUE(map.Contains(loc.coord));
        EXPECT_TRUE(registry.terrain_registry.IsPassable(map.At(loc.coord).terrain_id));
    }
}

TEST(WorldGenerator, SameSeedGeneratesAnIdenticalWorld) {
    EXPECT_EQ(Snapshot(20260729U), Snapshot(20260729U));
}

TEST(WorldGenerator, DifferentSeedsGenerateDifferentWorlds) {
    EXPECT_NE(Snapshot(1U), Snapshot(2U));
}

TEST(WorldGenerator, GuaranteesOneShopkeeperPerLevelRegardlessOfRandomRoll) {
    domain::Registry registry;
    auto config = MakeConfig();
    auto db = MakeDatabase();
    db.AddMonster({.id = "shopkeeper",
                   .name = "Shopkeeper",
                   .map_level = 3,
                   .view_zone = 4,
                   .max_hp = 20,
                   .chars = {0, 0, 0, 0, 0},
                   .tile = "../shopkeeper.bmp",
                   .shopkeeper = true});

    roll::Seed(1U);
    GenerateWorldForTest(registry, config, db);

    for (int level = 0; level < config.map_config.max_level; level++) {
        int shopkeeper_count = 0;
        for (auto id : registry.View<component::ShopkeeperTag, Location>()) {
            if (registry.GetRef<Location>(id).map_level == level) {
                shopkeeper_count++;
            }
        }
        EXPECT_EQ(shopkeeper_count, 1) << "level " << level;
    }
}

TEST(WorldGenerator, GenerateMonstersGivesEachMonsterAPurse) {
    domain::Registry registry;
    auto config = MakeConfig();
    auto db = MakeDatabase();

    roll::Seed(1U);
    GenerateWorldForTest(registry, config, db);

    int checked = 0;
    for (const auto &[id, tag] : registry.Storage<component::MonsterTag>()) {
        (void)tag;
        EXPECT_TRUE(registry.Storage<component::PurseComponent>().Has(id));
        checked++;
    }
    EXPECT_GT(checked, 0);
}

TEST(WorldGenerator, GenerateMonstersNeverSpawnsOnTheHerosTile) {
    domain::Registry registry;
    const map::Cell free_cell{.terrain_id = 0, .explored = false};
    registry.maps.emplace_back(3, 1, free_cell);

    auto hero_id = registry.entities.CreateEntity();
    registry.Storage<component::PlayerTag>().Add(hero_id, {});
    registry.Storage<Location>().Add(hero_id, {.coord = {.x = 0, .y = 0}, .map_level = 0});

    auto config =
        domain::Config{.map_config = {.width = 3, .height = 1, .max_level = 1, .border_size = 0},
                       .num_monsters = 1,
                       .seed = std::nullopt};
    auto db = MakeDatabase();
    db.AddMonster({.id = "shopkeeper",
                   .name = "Shopkeeper",
                   .map_level = 0,
                   .view_zone = 4,
                   .max_hp = 20,
                   .chars = {0, 0, 0, 0, 0},
                   .tile = "../shopkeeper.bmp",
                   .shopkeeper = true});

    roll::Seed(1U);
    domain::WorldGenerator::GenerateMonsters(registry, db, config.num_monsters);

    int num_monsters = 0;
    for (const auto &[id, tag] : registry.Storage<component::MonsterTag>()) {
        (void)tag;
        num_monsters++;
        EXPECT_NE(registry.GetRef<Location>(id).coord, (Vector2Int{.x = 0, .y = 0}));
    }
    EXPECT_EQ(num_monsters, 2);
}

namespace {
domain::Registry MakeCrowdedWorld(int width) {
    domain::Registry registry;
    const map::Cell free_cell{.terrain_id = 0, .explored = false};
    registry.maps.emplace_back(width, 1, free_cell);

    auto hero_id = registry.entities.CreateEntity();
    registry.Storage<component::PlayerTag>().Add(hero_id, {});
    registry.Storage<Location>().Add(hero_id, {.coord = {.x = 0, .y = 0}, .map_level = 0});
    return registry;
}

domain::Config MakeCrowdedConfig(int width) {
    return domain::Config{
        .map_config = {.width = width, .height = 1, .max_level = 1, .border_size = 0},
        .num_monsters = 1,
        .seed = std::nullopt};
}
} // namespace

TEST(WorldGenerator, ThrowsRatherThanSilentlyPlacingFewerMonstersThanConfigured) {
    auto registry = MakeCrowdedWorld(1);
    auto config = MakeCrowdedConfig(1);
    auto db = MakeDatabase();

    roll::Seed(1U);
    EXPECT_THROW(domain::WorldGenerator::GenerateMonsters(registry, db, config.num_monsters),
                 std::runtime_error);
}

TEST(WorldGenerator, ThrowsRatherThanSilentlySkippingTheGuaranteedShopkeeper) {
    auto registry = MakeCrowdedWorld(2);
    auto config = MakeCrowdedConfig(2);
    auto db = MakeDatabase();
    db.AddMonster({.id = "shopkeeper",
                   .name = "Shopkeeper",
                   .map_level = 0,
                   .view_zone = 4,
                   .max_hp = 20,
                   .chars = {0, 0, 0, 0, 0},
                   .tile = "../shopkeeper.bmp",
                   .shopkeeper = true});

    roll::Seed(1U);
    EXPECT_THROW(domain::WorldGenerator::GenerateMonsters(registry, db, config.num_monsters),
                 std::runtime_error);
}

TEST(WorldGenerator, NoTwoEntitiesEverShareASpawnTile) {
    for (const unsigned int seed : {1U, 2U, 7U, 20260815U}) {
        domain::Registry registry;
        auto config = MakeConfig();
        auto db = MakeDatabase();
        db.AddMonster({.id = "shopkeeper",
                       .name = "Shopkeeper",
                       .map_level = 0,
                       .view_zone = 4,
                       .max_hp = 20,
                       .chars = {0, 0, 0, 0, 0},
                       .tile = "../shopkeeper.bmp",
                       .shopkeeper = true});

        roll::Seed(seed);
        GenerateWorldForTest(registry, config, db);

        std::vector<std::string> occupied;
        for (const auto &[id, loc] : registry.Storage<Location>()) {
            occupied.push_back(std::to_string(loc.coord.x) + "," + std::to_string(loc.coord.y) +
                               "@" + std::to_string(loc.map_level));
        }
        std::ranges::sort(occupied);
        const auto collision = std::ranges::adjacent_find(occupied);
        EXPECT_EQ(collision, occupied.end())
            << "seed " << seed << ": two entities on tile "
            << (collision == occupied.end() ? std::string{} : *collision);
    }
}

TEST(WorldGenerator, GenerateMonstersPlacesTheConfiguredCountOnPassableTerrain) {
    auto registry = MakeTestRegistry();

    auto config =
        domain::Config{.map_config = {.width = 10, .height = 10, .max_level = 1, .border_size = 2},
                       .num_monsters = 1,
                       .seed = std::nullopt};
    auto db = database::GameDatabase{};
    db.AddMonster({.id = "Rat",
                   .name = "STR_RAT",
                   .map_level = 0,
                   .view_zone = 4,
                   .max_hp = 3,
                   .chars = {1, 1, 0, 0, 0},
                   .tile = "../rat.bmp"});
    domain::WorldGenerator::GenerateMonsters(registry, db, config.num_monsters);

    int num_monsters = 0;
    for (const auto &[_monster_id, _monster_tag] : registry.Storage<component::MonsterTag>()) {
        (void)_monster_id;
        (void)_monster_tag;
        num_monsters += 1;
    }
    ASSERT_EQ(num_monsters, 1);
    for (const auto &[monster_id, monster_tag] : registry.Storage<component::MonsterTag>()) {
        const auto *loc = registry.Storage<Location>().Get(monster_id);
        ASSERT_TRUE(registry.terrain_registry.IsPassable(
            registry.maps[loc->map_level].At(loc->coord).terrain_id));
    }
}
