#include "Components.h"
#include "EntityFactory.h"
#include "EntityStorage.h"
#include "Event.h"
#include "GameItem.h"
#include "Inventory.h"
#include "Location.h"
#include "Registry.h"
#include "Support/Fixtures.h"
#include "Vector2Int.h"
#include <array>
#include <gtest/gtest.h>
#include <optional>
#include <stdexcept>

namespace {
entity::EntityId MakeGroundCurrency(domain::Registry &registry, Vector2Int coord, int amount) {
    auto id = MakeGroundItemAt(registry, {.coord = coord, .map_level = 0}, 0);
    registry.Storage<game_item::CurrencyComponent>().Add(id, {.amount = amount});
    return id;
}
} // namespace

TEST(Inventory, PickUpMovesItemFromGroundToOwnership) {
    domain::Registry registry;
    auto actor = MakeEntityAt(registry, {.coord = {.x = 1, .y = 1}, .map_level = 0});
    auto item = MakeGroundItemAt(registry, {.coord = {.x = 1, .y = 1}, .map_level = 0}, 5);

    EXPECT_TRUE(game_item::PickUp(registry, {.item = item, .actor = actor}));

    EXPECT_FALSE(registry.Storage<Location>().Has(item));
    ASSERT_TRUE(registry.Storage<game_item::OwnershipComponent>().Has(item));
    EXPECT_EQ(registry.Storage<game_item::OwnershipComponent>().Get(item)->owner, actor);
}

TEST(Inventory, DropRestoresLocationAtActorCell) {
    domain::Registry registry;
    auto actor = MakeEntityAt(registry, {.coord = {.x = 2, .y = 3}, .map_level = 0});
    auto item = MakeCarriedItem(registry, actor, 5);

    EXPECT_TRUE(game_item::Drop(registry, {.item = item, .actor = actor}));

    EXPECT_FALSE(registry.Storage<game_item::OwnershipComponent>().Has(item));
    ASSERT_TRUE(registry.Storage<Location>().Has(item));
    EXPECT_EQ(registry.Storage<Location>().Get(item)->coord, (Vector2Int{.x = 2, .y = 3}));
}

TEST(Inventory, PickUpRejectsWhenOverCapacity) {
    domain::Registry registry;
    auto actor = MakeEntityAt(registry, {.coord = {.x = 1, .y = 1}, .map_level = 0});
    MakeCarriedItem(registry, actor, game_item::kMaxCarrySize);
    auto item = MakeGroundItemAt(registry, {.coord = {.x = 1, .y = 1}, .map_level = 0}, 1);

    EXPECT_FALSE(game_item::PickUp(registry, {.item = item, .actor = actor}));

    EXPECT_TRUE(registry.Storage<Location>().Has(item));
    EXPECT_FALSE(registry.Storage<game_item::OwnershipComponent>().Has(item));
}

TEST(Inventory, OwnedItemsReturnsOnlyThatActorsItems) {
    domain::Registry registry;
    auto actor = MakeEntityAt(registry, {.coord = {.x = 1, .y = 1}, .map_level = 0});
    auto other = MakeEntityAt(registry, {.coord = {.x = 5, .y = 5}, .map_level = 0});
    auto mine = MakeCarriedItem(registry, actor, 3);
    MakeCarriedItem(registry, other, 4);

    auto owned = game_item::OwnedItems(registry, actor);

    ASSERT_EQ(owned.size(), 1U);
    EXPECT_EQ(owned.front(), mine);
}

TEST(Inventory, PickUpRejectsAlreadyOwnedItem) {
    domain::Registry registry;
    auto owner = MakeEntityAt(registry, {.coord = {.x = 1, .y = 1}, .map_level = 0});
    auto actor = MakeEntityAt(registry, {.coord = {.x = 1, .y = 1}, .map_level = 0});
    auto item = MakeCarriedItem(registry, owner, 1);

    EXPECT_FALSE(game_item::PickUp(registry, {.item = item, .actor = actor}));

    EXPECT_EQ(registry.Storage<game_item::OwnershipComponent>().Get(item)->owner, owner);
}

TEST(Inventory, PickUpCurrencyAddsToPurseAndDestroysItem) {
    domain::Registry registry;
    auto actor = MakeEntityAt(registry, {.coord = {.x = 1, .y = 1}, .map_level = 0});
    auto coins = MakeGroundCurrency(registry, {.x = 1, .y = 1}, 7);

    EXPECT_TRUE(game_item::PickUp(registry, {.item = coins, .actor = actor}));

    EXPECT_EQ(game_item::Money(registry, actor), 7);
    EXPECT_FALSE(registry.Storage<game_item::CurrencyComponent>().Has(coins));
    EXPECT_FALSE(registry.Storage<Location>().Has(coins));
}

TEST(Inventory, PickUpCurrencyAddsToExistingPurse) {
    domain::Registry registry;
    auto actor = MakeEntityAt(registry, {.coord = {.x = 1, .y = 1}, .map_level = 0});
    registry.Storage<component::PurseComponent>().Add(actor, {.money = 3});
    auto coins = MakeGroundCurrency(registry, {.x = 1, .y = 1}, 7);

    EXPECT_TRUE(game_item::PickUp(registry, {.item = coins, .actor = actor}));

    EXPECT_EQ(game_item::Money(registry, actor), 10);
}

TEST(Inventory, PickUpNonCurrencyItemIsStoredNormally) {
    domain::Registry registry;
    auto actor = MakeEntityAt(registry, {.coord = {.x = 1, .y = 1}, .map_level = 0});
    auto sword = MakeGroundItemAt(registry, {.coord = {.x = 1, .y = 1}, .map_level = 0}, 3);

    EXPECT_TRUE(game_item::PickUp(registry, {.item = sword, .actor = actor}));

    EXPECT_EQ(game_item::Money(registry, actor), 0);
    ASSERT_TRUE(registry.Storage<game_item::OwnershipComponent>().Has(sword));
    EXPECT_EQ(registry.Storage<game_item::OwnershipComponent>().Get(sword)->owner, actor);
}

TEST(Inventory, PickUpFromTilePicksUpTheItemOnActorsTile) {
    domain::Registry registry;
    auto actor = MakeEntityAt(registry, {.coord = {.x = 1, .y = 1}, .map_level = 0});
    auto item = MakeGroundItemAt(registry, {.coord = {.x = 1, .y = 1}, .map_level = 0}, 3);
    registry.Storage<component::ItemTag>().Add(item, {});
    registry.Storage<component::NameComponent>().Add(item, {.name = "a dagger"});

    auto result = game_item::PickUpFromTile(registry, actor);

    ASSERT_TRUE(registry.Storage<game_item::OwnershipComponent>().Has(item));
    EXPECT_EQ(registry.Storage<game_item::OwnershipComponent>().Get(item)->owner, actor);
    ASSERT_EQ(result.events.size(), 1U);
    EXPECT_EQ(result.events.front().type, event::Type::ItemPickedUp);
    EXPECT_EQ(result.events.front().defender_name, "a dagger");
}

TEST(Inventory, PickUpFromTileEmitsNoItemWhenTileIsEmpty) {
    domain::Registry registry;
    auto actor = MakeEntityAt(registry, {.coord = {.x = 1, .y = 1}, .map_level = 0});

    auto result = game_item::PickUpFromTile(registry, actor);

    ASSERT_EQ(result.events.size(), 1U);
    EXPECT_EQ(result.events.front().type, event::Type::NoItem);
}

TEST(Inventory, PickUpFromTileEmitsFullInventoryWhenOverCapacity) {
    domain::Registry registry;
    auto actor = MakeEntityAt(registry, {.coord = {.x = 1, .y = 1}, .map_level = 0});
    MakeCarriedItem(registry, actor, game_item::kMaxCarrySize);
    auto item = MakeGroundItemAt(registry, {.coord = {.x = 1, .y = 1}, .map_level = 0}, 1);
    registry.Storage<component::ItemTag>().Add(item, {});

    auto result = game_item::PickUpFromTile(registry, actor);

    EXPECT_TRUE(registry.Storage<Location>().Has(item));
    EXPECT_FALSE(registry.Storage<game_item::OwnershipComponent>().Has(item));
    ASSERT_EQ(result.events.size(), 1U);
    EXPECT_EQ(result.events.front().type, event::Type::FullInventory);
}

TEST(Inventory, MoneyIsZeroWhenNeverPickedUp) {
    domain::Registry registry;
    auto actor = MakeEntityAt(registry, {.coord = {.x = 1, .y = 1}, .map_level = 0});

    EXPECT_EQ(game_item::Money(registry, actor), 0);
}

TEST(Inventory, EquipIntoAllowedSlot) {
    domain::Registry registry;
    auto actor = MakeEntityAt(registry, {.coord = {.x = 1, .y = 1}, .map_level = 0});
    auto sword = MakeCarriedEquippableItem(registry, actor, game_item::EquipSlot::AnyHand);

    EXPECT_TRUE(game_item::Equip(registry, {.item = sword, .actor = actor},
                                 game_item::EquipSlot::MainHand));

    ASSERT_TRUE(registry.Storage<game_item::EquippedComponent>().Has(sword));
    EXPECT_EQ(registry.Storage<game_item::EquippedComponent>().Get(sword)->slot,
              game_item::EquipSlot::MainHand);
}

TEST(Inventory, EquipRejectsDisallowedSlot) {
    domain::Registry registry;
    auto actor = MakeEntityAt(registry, {.coord = {.x = 1, .y = 1}, .map_level = 0});
    auto helm = MakeCarriedEquippableItem(registry, actor, game_item::EquipSlot::Head);

    EXPECT_FALSE(
        game_item::Equip(registry, {.item = helm, .actor = actor}, game_item::EquipSlot::MainHand));

    EXPECT_FALSE(registry.Storage<game_item::EquippedComponent>().Has(helm));
}

TEST(Inventory, EquipRejectsOccupiedSlot) {
    domain::Registry registry;
    auto actor = MakeEntityAt(registry, {.coord = {.x = 1, .y = 1}, .map_level = 0});
    auto first_sword = MakeCarriedEquippableItem(registry, actor, game_item::EquipSlot::AnyHand);
    auto second_sword = MakeCarriedEquippableItem(registry, actor, game_item::EquipSlot::AnyHand);
    ASSERT_TRUE(game_item::Equip(registry, {.item = first_sword, .actor = actor},
                                 game_item::EquipSlot::MainHand));

    EXPECT_FALSE(game_item::Equip(registry, {.item = second_sword, .actor = actor},
                                  game_item::EquipSlot::MainHand));

    EXPECT_FALSE(registry.Storage<game_item::EquippedComponent>().Has(second_sword));
}

TEST(Inventory, UnequipLeavesItemCarried) {
    domain::Registry registry;
    auto actor = MakeEntityAt(registry, {.coord = {.x = 1, .y = 1}, .map_level = 0});
    auto sword = MakeCarriedEquippableItem(registry, actor, game_item::EquipSlot::AnyHand);
    ASSERT_TRUE(game_item::Equip(registry, {.item = sword, .actor = actor},
                                 game_item::EquipSlot::MainHand));

    game_item::Unequip(registry, sword);

    EXPECT_FALSE(registry.Storage<game_item::EquippedComponent>().Has(sword));
    ASSERT_TRUE(registry.Storage<game_item::OwnershipComponent>().Has(sword));
    EXPECT_EQ(registry.Storage<game_item::OwnershipComponent>().Get(sword)->owner, actor);
}

TEST(Inventory, DropUnequipsItemFirst) {
    domain::Registry registry;
    auto actor = MakeEntityAt(registry, {.coord = {.x = 1, .y = 1}, .map_level = 0});
    auto sword = MakeCarriedEquippableItem(registry, actor, game_item::EquipSlot::AnyHand);
    ASSERT_TRUE(game_item::Equip(registry, {.item = sword, .actor = actor},
                                 game_item::EquipSlot::MainHand));

    EXPECT_TRUE(game_item::Drop(registry, {.item = sword, .actor = actor}));

    EXPECT_FALSE(registry.Storage<game_item::EquippedComponent>().Has(sword));
    EXPECT_TRUE(registry.Storage<Location>().Has(sword));
}

TEST(Inventory, EquipComboSlotAsTargetThrows) {
    domain::Registry registry;
    auto actor = MakeEntityAt(registry, {.coord = {.x = 1, .y = 1}, .map_level = 0});
    auto ring = MakeCarriedEquippableItem(registry, actor, game_item::EquipSlot::AnyRing);

    EXPECT_THROW(
        game_item::Equip(registry, {.item = ring, .actor = actor}, game_item::EquipSlot::AnyRing),
        std::invalid_argument);
    EXPECT_FALSE(registry.Storage<game_item::EquippedComponent>().Has(ring));
}

TEST(Inventory, EquipNoneAsTargetThrows) {
    domain::Registry registry;
    auto actor = MakeEntityAt(registry, {.coord = {.x = 1, .y = 1}, .map_level = 0});
    auto item = MakeCarriedEquippableItem(registry, actor, game_item::EquipSlot::AnyHand);

    EXPECT_THROW(
        game_item::Equip(registry, {.item = item, .actor = actor}, game_item::EquipSlot::None),
        std::invalid_argument);
}

TEST(CreateItem, TemplateWithBothRangedAndAmmoThrows) {
    domain::Registry registry;
    const game_item::ItemTemplate prefab{
        .id = "confused_crossbow",
        .name = "Confused Crossbow",
        .melee = std::nullopt,
        .ranged = game_item::RangedWeaponComponent{.damage = {.count = 1, .sides = 6},
                                                   .max_ammo = 5,
                                                   .range = 4},
        .armor = std::nullopt,
        .ammo = game_item::AmmoComponent{.amount = 20},
        .magic = std::nullopt,
        .equippable = std::nullopt,
        .currency = std::nullopt,
        .value = std::nullopt,
        .size = 2,
        .sprite_name = "../crossbow.bmp"};

    EXPECT_THROW(entity::CreateItem(registry, prefab,
                                    Location{.coord = {.x = 0, .y = 0}, .map_level = 0},
                                    std::nullopt),
                 std::invalid_argument);
}

TEST(Inventory, EquippedItemsIsTheWornSubsetOfOwnedItems) {
    domain::Registry registry;
    auto actor = MakeEntityAt(registry, {.coord = {.x = 1, .y = 1}, .map_level = 0});
    auto worn = MakeCarriedEquippableItem(registry, actor, game_item::EquipSlot::AnyHand);
    MakeCarriedItem(registry, actor, 1);
    auto other_actor = MakeEntityAt(registry, {.coord = {.x = 5, .y = 5}, .map_level = 0});
    MakeEquippedItem(registry, other_actor, game_item::EquipSlot::Head);
    ASSERT_TRUE(
        game_item::Equip(registry, {.item = worn, .actor = actor}, game_item::EquipSlot::MainHand));

    auto equipped = game_item::EquippedItems(registry, actor);

    ASSERT_EQ(equipped.size(), 1U);
    EXPECT_EQ(equipped.front(), worn);
}

TEST(Inventory, EquippedInSlotFindsOnlyThatSlot) {
    domain::Registry registry;
    auto actor = MakeEntityAt(registry, {.coord = {.x = 1, .y = 1}, .map_level = 0});
    auto helm = MakeEquippedItem(registry, actor, game_item::EquipSlot::Head);

    EXPECT_EQ(game_item::EquippedInSlot(registry, actor, game_item::EquipSlot::Head), helm);
    EXPECT_FALSE(
        game_item::EquippedInSlot(registry, actor, game_item::EquipSlot::MainHand).has_value());
}

TEST(Inventory, AddMoneyCreatesThePurseOnFirstCoinAndAccumulates) {
    domain::Registry registry;
    auto actor = MakeEntityAt(registry, {.coord = {.x = 1, .y = 1}, .map_level = 0});

    game_item::AddMoney(registry, actor, 7);
    EXPECT_EQ(game_item::Money(registry, actor), 7);

    game_item::AddMoney(registry, actor, 3);
    EXPECT_EQ(game_item::Money(registry, actor), 10);

    game_item::AddMoney(registry, actor, -4);
    EXPECT_EQ(game_item::Money(registry, actor), 6);
}

TEST(Inventory, TransferItemReassignsOwnerAndUnequips) {
    domain::Registry registry;
    auto seller = MakeEntityAt(registry, {.coord = {.x = 1, .y = 1}, .map_level = 0});
    auto buyer = MakeEntityAt(registry, {.coord = {.x = 2, .y = 2}, .map_level = 0});
    auto sword = MakeEquippedItem(registry, seller, game_item::EquipSlot::MainHand);

    EXPECT_TRUE(game_item::TransferItem(registry, {.item = sword, .actor = buyer}));

    EXPECT_EQ(registry.GetRef<game_item::OwnershipComponent>(sword).owner, buyer);
    EXPECT_FALSE(registry.Storage<game_item::EquippedComponent>().Has(sword));
}

TEST(Inventory, TransferItemRejectsAnItemOnTheGround) {
    domain::Registry registry;
    auto actor = MakeEntityAt(registry, {.coord = {.x = 1, .y = 1}, .map_level = 0});
    auto loose = MakeGroundItemAt(registry, {.coord = {.x = 1, .y = 1}, .map_level = 0}, 1);

    EXPECT_FALSE(game_item::TransferItem(registry, {.item = loose, .actor = actor}));

    EXPECT_FALSE(registry.Storage<game_item::OwnershipComponent>().Has(loose));
}

TEST(Inventory, UseItemStampsIntentAndDecrementsCharges) {
    domain::Registry registry;
    auto actor = MakeEntityAt(registry, {.coord = {.x = 1, .y = 1}, .map_level = 0});
    auto staff = MakeCarriedMagicItem(registry, actor, "fireball", 3);

    EXPECT_TRUE(game_item::UseItem(registry, {.item = staff, .actor = actor}));

    ASSERT_TRUE(registry.Storage<component::IntentToCastSpell>().Has(actor));
    const auto &intent = *registry.Storage<component::IntentToCastSpell>().Get(actor);
    EXPECT_EQ(intent.spell_id, "fireball");
    EXPECT_FALSE(intent.costs_mana);
    EXPECT_FALSE(intent.direction.has_value());
    ASSERT_TRUE(registry.Storage<game_item::ChargesComponent>().Has(staff));
    EXPECT_EQ(registry.Storage<game_item::ChargesComponent>().Get(staff)->amount, 2);
}

TEST(Inventory, UseItemDestroysAndUnequipsAtZeroCharges) {
    domain::Registry registry;
    auto actor = MakeEntityAt(registry, {.coord = {.x = 1, .y = 1}, .map_level = 0});
    auto potion = MakeCarriedMagicItem(registry, actor, "healing_light", 1);
    registry.Storage<game_item::EquippedComponent>().Add(potion,
                                                         {.slot = game_item::EquipSlot::MainHand});

    EXPECT_TRUE(game_item::UseItem(registry, {.item = potion, .actor = actor}));

    ASSERT_TRUE(registry.Storage<component::IntentToCastSpell>().Has(actor));
    EXPECT_FALSE(registry.Storage<game_item::OwnershipComponent>().Has(potion));
    EXPECT_FALSE(registry.Storage<game_item::EquippedComponent>().Has(potion));
}

TEST(Inventory, UseItemRejectsNonOwner) {
    domain::Registry registry;
    auto owner = MakeEntityAt(registry, {.coord = {.x = 1, .y = 1}, .map_level = 0});
    auto other = MakeEntityAt(registry, {.coord = {.x = 2, .y = 2}, .map_level = 0});
    auto staff = MakeCarriedMagicItem(registry, owner, "fireball", 3);

    EXPECT_FALSE(game_item::UseItem(registry, {.item = staff, .actor = other}));

    EXPECT_FALSE(registry.Storage<component::IntentToCastSpell>().Has(other));
    EXPECT_EQ(registry.Storage<game_item::ChargesComponent>().Get(staff)->amount, 3);
}

TEST(Inventory, UseItemRejectsNonMagicItem) {
    domain::Registry registry;
    auto actor = MakeEntityAt(registry, {.coord = {.x = 1, .y = 1}, .map_level = 0});
    auto sword = MakeCarriedItem(registry, actor, 1);

    EXPECT_FALSE(game_item::UseItem(registry, {.item = sword, .actor = actor}));

    EXPECT_FALSE(registry.Storage<component::IntentToCastSpell>().Has(actor));
}

TEST(Inventory, UnequippedItemsIsTheCarriedButNotWornSubsetOfOwnedItems) {
    domain::Registry registry;
    auto actor = MakeEntityAt(registry, {.coord = {.x = 1, .y = 1}, .map_level = 0});
    auto worn = MakeCarriedEquippableItem(registry, actor, game_item::EquipSlot::AnyHand);
    auto stowed = MakeCarriedItem(registry, actor, 1);
    auto other_actor = MakeEntityAt(registry, {.coord = {.x = 5, .y = 5}, .map_level = 0});
    MakeCarriedItem(registry, other_actor, 1);
    ASSERT_TRUE(
        game_item::Equip(registry, {.item = worn, .actor = actor}, game_item::EquipSlot::MainHand));

    auto unequipped = game_item::UnequippedItems(registry, actor);

    ASSERT_EQ(unequipped.size(), 1U);
    EXPECT_EQ(unequipped.front(), stowed);
    EXPECT_EQ(unequipped.size() + game_item::EquippedItems(registry, actor).size(),
              game_item::OwnedItems(registry, actor).size());
}

TEST(Inventory, EquipToFirstFittingSlotHonoursTheCallersSlotOrder) {
    domain::Registry registry;
    auto actor = MakeEntityAt(registry, {.coord = {.x = 1, .y = 1}, .map_level = 0});
    auto blade = MakeCarriedEquippableItem(registry, actor, game_item::EquipSlot::AnyHand);

    constexpr std::array<game_item::EquipSlot, 2> kOffHandFirst = {game_item::EquipSlot::OffHand,
                                                                   game_item::EquipSlot::MainHand};
    ASSERT_TRUE(game_item::EquipToFirstFittingSlot(registry, {.item = blade, .actor = actor},
                                                   kOffHandFirst));
    EXPECT_EQ(registry.GetRef<game_item::EquippedComponent>(blade).slot,
              game_item::EquipSlot::OffHand);
}

TEST(Inventory, EquipToFirstFittingSlotSkipsOccupiedSlots) {
    domain::Registry registry;
    auto actor = MakeEntityAt(registry, {.coord = {.x = 1, .y = 1}, .map_level = 0});
    auto first = MakeCarriedEquippableItem(registry, actor, game_item::EquipSlot::AnyHand);
    auto second = MakeCarriedEquippableItem(registry, actor, game_item::EquipSlot::AnyHand);

    constexpr std::array<game_item::EquipSlot, 2> kHands = {game_item::EquipSlot::MainHand,
                                                            game_item::EquipSlot::OffHand};
    ASSERT_TRUE(
        game_item::EquipToFirstFittingSlot(registry, {.item = first, .actor = actor}, kHands));
    ASSERT_TRUE(
        game_item::EquipToFirstFittingSlot(registry, {.item = second, .actor = actor}, kHands));

    EXPECT_EQ(registry.GetRef<game_item::EquippedComponent>(first).slot,
              game_item::EquipSlot::MainHand);
    EXPECT_EQ(registry.GetRef<game_item::EquippedComponent>(second).slot,
              game_item::EquipSlot::OffHand);
}

TEST(Inventory, EquipToFirstFittingSlotFailsOnANonEquippableItemAndWhenEverySlotIsTaken) {
    domain::Registry registry;
    auto actor = MakeEntityAt(registry, {.coord = {.x = 1, .y = 1}, .map_level = 0});
    constexpr std::array<game_item::EquipSlot, 1> kMainHandOnly = {game_item::EquipSlot::MainHand};

    auto plain = MakeCarriedItem(registry, actor, 1);
    EXPECT_FALSE(game_item::EquipToFirstFittingSlot(registry, {.item = plain, .actor = actor},
                                                    kMainHandOnly));

    auto held = MakeCarriedEquippableItem(registry, actor, game_item::EquipSlot::AnyHand);
    auto spare = MakeCarriedEquippableItem(registry, actor, game_item::EquipSlot::AnyHand);
    ASSERT_TRUE(game_item::EquipToFirstFittingSlot(registry, {.item = held, .actor = actor},
                                                   kMainHandOnly));
    EXPECT_FALSE(game_item::EquipToFirstFittingSlot(registry, {.item = spare, .actor = actor},
                                                    kMainHandOnly));
    EXPECT_FALSE(registry.Storage<game_item::EquippedComponent>().Has(spare));
}
