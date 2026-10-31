#include "Components.h"
#include "EntityStorage.h"
#include "GameItem.h"
#include "Inventory.h"
#include "Registry.h"
#include "Support/Fixtures.h"
#include "Trade.h"
#include <gtest/gtest.h>

namespace {
entity::EntityId MakeHeroWithCha(domain::Registry &registry, int cha) {
    auto id = registry.entities.CreateEntity();
    registry.Storage<component::AttributesComponent>().Add(id, WithCha(cha));
    return id;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
entity::EntityId MakeOwnedItem(domain::Registry &registry, entity::EntityId owner, int base_price,
                               int size = 1) {
    auto id = registry.entities.CreateEntity();
    registry.Storage<game_item::OwnershipComponent>().Add(id, {.owner = owner});
    registry.Storage<game_item::ValueComponent>().Add(id, {.base_price = base_price});
    registry.Storage<game_item::SizeComponent>().Add(id, {.size = size});
    return id;
}
} // namespace

TEST(Trade, HagglePriceFavorsHeroOnSuccessRegardlessOfSide) {
    EXPECT_EQ(trade::HagglePrice(100, /*success=*/true, /*hero_is_buying=*/true), 90);
    EXPECT_EQ(trade::HagglePrice(100, /*success=*/false, /*hero_is_buying=*/true), 110);
    EXPECT_EQ(trade::HagglePrice(100, /*success=*/true, /*hero_is_buying=*/false), 110);
    EXPECT_EQ(trade::HagglePrice(100, /*success=*/false, /*hero_is_buying=*/false), 90);
}

TEST(Trade, SellTransfersItemAndMoneyOnSuccessfulHaggle) {
    domain::Registry registry;
    auto hero_id = MakeHeroWithCha(registry, 100);
    auto shopkeeper_id = registry.entities.CreateEntity();
    registry.Storage<component::PurseComponent>().Add(shopkeeper_id, {.money = 1000});
    auto item_id = MakeOwnedItem(registry, hero_id, 100);

    EXPECT_TRUE(trade::Sell(
        registry, {.hero_id = hero_id, .shopkeeper_id = shopkeeper_id, .item_id = item_id}));

    EXPECT_EQ(registry.Storage<game_item::OwnershipComponent>().Get(item_id)->owner, shopkeeper_id);
    EXPECT_EQ(game_item::Money(registry, hero_id), 110);
    EXPECT_EQ(game_item::Money(registry, shopkeeper_id), 1000 - 110);
}

TEST(Trade, SellRejectsItemNotOwnedByHero) {
    domain::Registry registry;
    auto hero_id = MakeHeroWithCha(registry, 100);
    auto shopkeeper_id = registry.entities.CreateEntity();
    registry.Storage<component::PurseComponent>().Add(shopkeeper_id, {.money = 1000});
    auto other_id = registry.entities.CreateEntity();
    auto item_id = MakeOwnedItem(registry, other_id, 100);

    EXPECT_FALSE(trade::Sell(
        registry, {.hero_id = hero_id, .shopkeeper_id = shopkeeper_id, .item_id = item_id}));

    EXPECT_EQ(registry.Storage<game_item::OwnershipComponent>().Get(item_id)->owner, other_id);
    EXPECT_EQ(game_item::Money(registry, hero_id), 0);
}

TEST(Trade, SellRejectsWhenShopkeeperCannotAfford) {
    domain::Registry registry;
    auto hero_id = MakeHeroWithCha(registry, 100);
    auto shopkeeper_id = registry.entities.CreateEntity();
    registry.Storage<component::PurseComponent>().Add(shopkeeper_id, {.money = 5});
    auto item_id = MakeOwnedItem(registry, hero_id, 100);

    EXPECT_FALSE(trade::Sell(
        registry, {.hero_id = hero_id, .shopkeeper_id = shopkeeper_id, .item_id = item_id}));

    EXPECT_EQ(registry.Storage<game_item::OwnershipComponent>().Get(item_id)->owner, hero_id);
    EXPECT_EQ(game_item::Money(registry, shopkeeper_id), 5);
}

TEST(Trade, BuyTransfersItemAndMoneyOnSuccessfulHaggle) {
    domain::Registry registry;
    auto hero_id = MakeHeroWithCha(registry, 100);
    registry.Storage<component::PurseComponent>().Add(hero_id, {.money = 1000});
    auto shopkeeper_id = registry.entities.CreateEntity();
    auto item_id = MakeOwnedItem(registry, shopkeeper_id, 100);

    EXPECT_TRUE(trade::Buy(
        registry, {.hero_id = hero_id, .shopkeeper_id = shopkeeper_id, .item_id = item_id}));

    EXPECT_EQ(registry.Storage<game_item::OwnershipComponent>().Get(item_id)->owner, hero_id);
    EXPECT_EQ(game_item::Money(registry, hero_id), 1000 - 90);
    EXPECT_EQ(game_item::Money(registry, shopkeeper_id), 90);
}

TEST(Trade, BuyRejectsItemNotOwnedByShopkeeper) {
    domain::Registry registry;
    auto hero_id = MakeHeroWithCha(registry, 100);
    registry.Storage<component::PurseComponent>().Add(hero_id, {.money = 1000});
    auto shopkeeper_id = registry.entities.CreateEntity();
    auto other_id = registry.entities.CreateEntity();
    auto item_id = MakeOwnedItem(registry, other_id, 100);

    EXPECT_FALSE(trade::Buy(
        registry, {.hero_id = hero_id, .shopkeeper_id = shopkeeper_id, .item_id = item_id}));

    EXPECT_EQ(registry.Storage<game_item::OwnershipComponent>().Get(item_id)->owner, other_id);
}

TEST(Trade, BuyRejectsWhenHeroCannotAfford) {
    domain::Registry registry;
    auto hero_id = MakeHeroWithCha(registry, 100);
    auto shopkeeper_id = registry.entities.CreateEntity();
    auto item_id = MakeOwnedItem(registry, shopkeeper_id, 100);

    EXPECT_FALSE(trade::Buy(
        registry, {.hero_id = hero_id, .shopkeeper_id = shopkeeper_id, .item_id = item_id}));

    EXPECT_EQ(registry.Storage<game_item::OwnershipComponent>().Get(item_id)->owner, shopkeeper_id);
    EXPECT_EQ(game_item::Money(registry, shopkeeper_id), 0);
}

TEST(Trade, BuyRejectsWhenHeroHasNoCarryCapacity) {
    domain::Registry registry;
    auto hero_id = MakeHeroWithCha(registry, 100);
    registry.Storage<component::PurseComponent>().Add(hero_id, {.money = 1000});
    MakeOwnedItem(registry, hero_id, /*base_price=*/1, /*size=*/game_item::kMaxCarrySize);
    auto shopkeeper_id = registry.entities.CreateEntity();
    auto item_id = MakeOwnedItem(registry, shopkeeper_id, 100, /*size=*/1);

    EXPECT_FALSE(trade::Buy(
        registry, {.hero_id = hero_id, .shopkeeper_id = shopkeeper_id, .item_id = item_id}));

    EXPECT_EQ(registry.Storage<game_item::OwnershipComponent>().Get(item_id)->owner, shopkeeper_id);
}

TEST(Trade, SellIsNotBlockedByShopkeeperCapacity) {
    domain::Registry registry;
    auto hero_id = MakeHeroWithCha(registry, 0);
    auto shopkeeper_id = registry.entities.CreateEntity();
    registry.Storage<component::PurseComponent>().Add(shopkeeper_id, {.money = 1000});
    MakeOwnedItem(registry, shopkeeper_id, /*base_price=*/1, /*size=*/game_item::kMaxCarrySize * 3);
    auto item_id = MakeOwnedItem(registry, hero_id, /*base_price=*/10, /*size=*/5);

    EXPECT_TRUE(trade::Sell(
        registry, {.hero_id = hero_id, .shopkeeper_id = shopkeeper_id, .item_id = item_id}));

    EXPECT_EQ(registry.GetRef<game_item::OwnershipComponent>(item_id).owner, shopkeeper_id);
}
