#pragma once
#include "EntityStorage.h"
#include "Registry.h"

namespace trade {

int HagglePrice(int base_price, bool success, bool hero_is_buying);

struct TradeArgs {
    entity::EntityId hero_id = 0;
    entity::EntityId shopkeeper_id = 0;
    entity::EntityId item_id = 0;
};

bool Sell(domain::Registry &registry, TradeArgs args);

bool Buy(domain::Registry &registry, TradeArgs args);

} // namespace trade
