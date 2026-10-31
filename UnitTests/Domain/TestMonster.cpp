#include "ComponentStorage.h"
#include "Components.h"
#include "EntityFactory.h"
#include "GameDatabase.h"
#include "GameItem.h"
#include "Inventory.h"
#include "Location.h"
#include "Monster.h"
#include "Support/TestData.h"
#include <gtest/gtest.h>
#include <optional>

TEST(Monster, CreateMonsterStampsEveryComponentFromThePrefab) {
    auto registry = domain::Registry{};

    const monster::MonsterPrefab prefab{.id = "TestMonster",
                                        .name = "MonsterName",
                                        .map_level = 3,
                                        .view_zone = 2,
                                        .max_hp = 5,
                                        .chars = {1, 1, 0, 0, 0},
                                        .tile = "monster_tile.bmp"};
    const Location loc{.coord = {.x = 10, .y = 20}, .map_level = 3};

    auto monster_id = entity::CreateMonster(registry, prefab, loc);

    EXPECT_EQ(registry.Storage<component::NameComponent>().Get(monster_id)->name, prefab.name);
    EXPECT_EQ(registry.Storage<component::VisionComponent>().Get(monster_id)->view_zone,
              prefab.view_zone);
    EXPECT_EQ(*registry.Storage<Location>().Get(monster_id), loc);
    EXPECT_TRUE(registry.Storage<component::MonsterTag>().Has(monster_id));
    EXPECT_FALSE(registry.Storage<component::PlayerTag>().Has(monster_id));
    EXPECT_EQ(registry.Storage<component::RenderComponent>().Get(monster_id)->tile, prefab.tile);
    EXPECT_EQ(registry.Storage<component::HealthComponent>().Get(monster_id)->current,
              prefab.max_hp);
    EXPECT_EQ(registry.Storage<component::HealthComponent>().Get(monster_id)->max, prefab.max_hp);
}

TEST(Monster, ShopkeeperPrefabStampsTagButNotStock) {
    auto registry = domain::Registry{};

    const monster::MonsterPrefab prefab{.id = "shopkeeper",
                                        .name = "Shopkeeper",
                                        .map_level = 3,
                                        .view_zone = 2,
                                        .max_hp = 5,
                                        .chars = {0, 0, 0, 0, 0},
                                        .tile = "shopkeeper.bmp",
                                        .shopkeeper = true};
    const Location loc{.coord = {.x = 10, .y = 20}, .map_level = 3};

    auto shopkeeper_id = entity::CreateMonster(registry, prefab, loc);

    EXPECT_TRUE(registry.Storage<component::ShopkeeperTag>().Has(shopkeeper_id));
    EXPECT_FALSE(registry.Storage<component::PurseComponent>().Has(shopkeeper_id));
    EXPECT_TRUE(game_item::OwnedItems(registry, shopkeeper_id).empty());
}

TEST(Monster, NonShopkeeperPrefabHasNoShopkeeperTag) {
    auto registry = domain::Registry{};

    const monster::MonsterPrefab prefab{.id = "TestMonster",
                                        .name = "MonsterName",
                                        .map_level = 3,
                                        .view_zone = 2,
                                        .max_hp = 5,
                                        .chars = {1, 1, 0, 0, 0},
                                        .tile = "monster_tile.bmp"};
    const Location loc{.coord = {.x = 10, .y = 20}, .map_level = 3};

    auto monster_id = entity::CreateMonster(registry, prefab, loc);

    EXPECT_FALSE(registry.Storage<component::ShopkeeperTag>().Has(monster_id));
}

TEST(Monster, GiveShopkeeperStockAddsPurseAndPricedItems) {
    auto registry = domain::Registry{};
    auto db = database::GameDatabase{};
    auto axe = MakeItemTemplate("axe_standard", "Axe", /*size=*/2, "../axe.bmp");
    axe.value = game_item::ValueComponent{.base_price = 10};
    db.AddItem(axe);
    auto copper = MakeItemTemplate("copper", "Copper", /*size=*/0, "../copper.bmp");
    copper.currency = game_item::CurrencyComponent{};
    db.AddItem(copper);

    const monster::MonsterPrefab prefab{.id = "shopkeeper",
                                        .name = "Shopkeeper",
                                        .map_level = 3,
                                        .view_zone = 2,
                                        .max_hp = 5,
                                        .chars = {0, 0, 0, 0, 0},
                                        .tile = "shopkeeper.bmp",
                                        .shopkeeper = true};
    const Location loc{.coord = {.x = 10, .y = 20}, .map_level = 3};
    auto shopkeeper_id = entity::CreateMonster(registry, prefab, loc);

    entity::GiveShopkeeperStock(registry, db, shopkeeper_id);

    ASSERT_TRUE(registry.Storage<component::PurseComponent>().Has(shopkeeper_id));
    EXPECT_GT(registry.Storage<component::PurseComponent>().Get(shopkeeper_id)->money, 0);

    auto owned = game_item::OwnedItems(registry, shopkeeper_id);
    ASSERT_EQ(owned.size(), 1);
    EXPECT_EQ(registry.Storage<component::NameComponent>().Get(owned[0])->name, "Axe");
}
