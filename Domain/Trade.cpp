#include "Trade.h"
#include "Components.h"
#include "EntityStorage.h"
#include "GameItem.h"
#include "Hero.h"
#include "Inventory.h"
#include "Registry.h"
#include "Roll.h"

namespace trade {
namespace {
constexpr int kHaggleChaScale = 5;
constexpr int kHagglePercent = 10;

bool RollsHaggle(const domain::Registry &registry, entity::EntityId hero_id) {
    const auto *attributes = registry.Storage<component::AttributesComponent>().Get(hero_id);
    const int cha = attributes != nullptr ? attributes->chars[hero::kChrCha] : 0;
    return roll::SkillCheck(cha * kHaggleChaScale, 0);
}

struct ExchangeArgs {
    entity::EntityId seller;
    entity::EntityId buyer;
    entity::EntityId item_id;
    entity::EntityId hero_id;
    bool hero_is_buying;
};

bool Exchange(domain::Registry &registry, ExchangeArgs args) {
    const auto *ownership = registry.Storage<game_item::OwnershipComponent>().Get(args.item_id);
    if (ownership == nullptr || ownership->owner != args.seller) {
        return false;
    }
    const auto *value = registry.Storage<game_item::ValueComponent>().Get(args.item_id);
    if (value == nullptr) {
        return false;
    }
    if (args.hero_is_buying) {
        const auto *size = registry.Storage<game_item::SizeComponent>().Get(args.item_id);
        if (size == nullptr ||
            game_item::CarriedSize(registry, args.buyer) + size->size > game_item::kMaxCarrySize) {
            return false;
        }
    }

    const int price =
        HagglePrice(value->base_price, RollsHaggle(registry, args.hero_id), args.hero_is_buying);
    if (game_item::Money(registry, args.buyer) < price) {
        return false;
    }

    game_item::TransferItem(registry, {.item = args.item_id, .actor = args.buyer});
    game_item::AddMoney(registry, args.buyer, -price);
    game_item::AddMoney(registry, args.seller, price);
    return true;
}
} // namespace

int HagglePrice(int base_price, bool success, bool hero_is_buying) {
    const int delta = (base_price * kHagglePercent) / 100;
    const bool price_drops = (success == hero_is_buying);
    return price_drops ? base_price - delta : base_price + delta;
}

bool Sell(domain::Registry &registry, TradeArgs args) {
    return Exchange(registry, {.seller = args.hero_id,
                               .buyer = args.shopkeeper_id,
                               .item_id = args.item_id,
                               .hero_id = args.hero_id,
                               .hero_is_buying = false});
}

bool Buy(domain::Registry &registry, TradeArgs args) {
    return Exchange(registry, {.seller = args.shopkeeper_id,
                               .buyer = args.hero_id,
                               .item_id = args.item_id,
                               .hero_id = args.hero_id,
                               .hero_is_buying = true});
}

} // namespace trade
