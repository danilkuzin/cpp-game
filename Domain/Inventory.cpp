#include "Inventory.h"
#include "Components.h"
#include "EntityStorage.h"
#include "Event.h"
#include "GameItem.h"
#include "Location.h"
#include "Registry.h"
#include "RegistryQueries.h"
#include <cstdint>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

namespace game_item {

std::vector<entity::EntityId> OwnedItems(const domain::Registry &registry, entity::EntityId actor) {
    std::vector<entity::EntityId> owned;
    for (auto item_id : registry.View<OwnershipComponent>()) {
        if (registry.GetRef<OwnershipComponent>(item_id).owner == actor) {
            owned.push_back(item_id);
        }
    }
    return owned;
}

int CarriedSize(const domain::Registry &registry, entity::EntityId actor) {
    int total = 0;
    for (auto item_id : OwnedItems(registry, actor)) {
        total += registry.GetRef<SizeComponent>(item_id).size;
    }
    return total;
}

std::vector<entity::EntityId> EquippedItems(const domain::Registry &registry,
                                            entity::EntityId actor) {
    std::vector<entity::EntityId> equipped;
    for (auto item_id : OwnedItems(registry, actor)) {
        if (registry.Storage<EquippedComponent>().Has(item_id)) {
            equipped.push_back(item_id);
        }
    }
    return equipped;
}

std::vector<entity::EntityId> UnequippedItems(const domain::Registry &registry,
                                              entity::EntityId actor) {
    std::vector<entity::EntityId> unequipped;
    for (auto item_id : OwnedItems(registry, actor)) {
        if (!registry.Storage<EquippedComponent>().Has(item_id)) {
            unequipped.push_back(item_id);
        }
    }
    return unequipped;
}

std::optional<entity::EntityId> EquippedInSlot(const domain::Registry &registry,
                                               entity::EntityId actor, EquipSlot slot) {
    for (auto item_id : EquippedItems(registry, actor)) {
        if (registry.GetRef<EquippedComponent>(item_id).slot == slot) {
            return item_id;
        }
    }
    return std::nullopt;
}

int Money(const domain::Registry &registry, entity::EntityId actor) {
    const auto *purse = registry.Storage<component::PurseComponent>().Get(actor);
    return purse != nullptr ? purse->money : 0;
}

void AddMoney(domain::Registry &registry, entity::EntityId actor, int amount) {
    auto *purse = registry.Storage<component::PurseComponent>().Get(actor);
    if (purse != nullptr) {
        purse->money += amount;
    } else {
        registry.Storage<component::PurseComponent>().Add(actor, {.money = amount});
    }
}

bool TransferItem(domain::Registry &registry, TransferItemArgs args) {
    auto *ownership = registry.Storage<OwnershipComponent>().Get(args.item);
    if (ownership == nullptr) {
        return false;
    }
    Unequip(registry, args.item);
    ownership->owner = args.actor;
    return true;
}

bool PickUp(domain::Registry &registry, TransferItemArgs args) {
    if (registry.Storage<OwnershipComponent>().Has(args.item)) {
        return false;
    }

    const auto *currency = registry.Storage<CurrencyComponent>().Get(args.item);
    if (currency != nullptr) {
        AddMoney(registry, args.actor, currency->amount);
        registry.DestroyEntity(args.item);
        return true;
    }

    const auto *size = registry.Storage<SizeComponent>().Get(args.item);
    if (size == nullptr || CarriedSize(registry, args.actor) + size->size > kMaxCarrySize) {
        return false;
    }

    registry.ClearLocation(args.item);
    registry.Storage<OwnershipComponent>().Add(args.item, {.owner = args.actor});
    return true;
}

event::Result PickUpFromTile(domain::Registry &registry, entity::EntityId actor) {
    event::Result result;
    const auto &hero_loc = registry.GetRef<Location>(actor);

    const auto item_id = systems::ItemIdAt(registry, hero_loc);
    if (!item_id.has_value()) {
        result.events.push_back(event::Event{.type = event::Type::NoItem});
        return result;
    }

    const auto *name = registry.Storage<component::NameComponent>().Get(item_id.value());
    const auto item_name = name != nullptr ? name->name : std::string{};

    if (!PickUp(registry, {.item = item_id.value(), .actor = actor})) {
        result.events.push_back(event::Event{.type = event::Type::FullInventory});
        return result;
    }
    result.events.push_back(
        event::Event{.type = event::Type::ItemPickedUp, .defender_name = item_name});

    return result;
}

bool Drop(domain::Registry &registry, TransferItemArgs args) {
    const auto *ownership = registry.Storage<OwnershipComponent>().Get(args.item);
    if (ownership == nullptr || ownership->owner != args.actor) {
        return false;
    }

    Unequip(registry, args.item);
    registry.Storage<OwnershipComponent>().Remove(args.item);
    registry.PlaceEntity(args.item, registry.GetRef<Location>(args.actor));
    return true;
}

bool Equip(domain::Registry &registry, TransferItemArgs args, EquipSlot target_slot) {
    if (!IsConcreteSlot(target_slot)) {
        throw std::invalid_argument("Equip target_slot must be a single concrete slot, got " +
                                    std::to_string(static_cast<uint32_t>(target_slot)));
    }
    const auto *ownership = registry.Storage<OwnershipComponent>().Get(args.item);
    if (ownership == nullptr || ownership->owner != args.actor) {
        return false;
    }
    const auto *equippable = registry.Storage<EquippableComponent>().Get(args.item);
    if (equippable == nullptr || !CanFitInSlot(equippable->allowed_slots, target_slot)) {
        return false;
    }
    if (EquippedInSlot(registry, args.actor, target_slot).has_value()) {
        return false;
    }

    registry.Storage<EquippedComponent>().Add(args.item, {.slot = target_slot});
    return true;
}

bool EquipToFirstFittingSlot(domain::Registry &registry, TransferItemArgs args,
                             std::span<const EquipSlot> candidate_slots) {
    const auto *equippable = registry.Storage<EquippableComponent>().Get(args.item);
    if (equippable == nullptr) {
        return false;
    }
    for (auto slot : candidate_slots) {
        if (CanFitInSlot(equippable->allowed_slots, slot) && Equip(registry, args, slot)) {
            return true;
        }
    }
    return false;
}

void Unequip(domain::Registry &registry, entity::EntityId item) {
    registry.Storage<EquippedComponent>().Remove(item);
}

bool UseItem(domain::Registry &registry, TransferItemArgs args) {
    const auto *ownership = registry.Storage<OwnershipComponent>().Get(args.item);
    if (ownership == nullptr || ownership->owner != args.actor) {
        return false;
    }
    const auto *magic = registry.Storage<MagicComponent>().Get(args.item);
    if (magic == nullptr) {
        return false;
    }
    const auto *charges = registry.Storage<ChargesComponent>().Get(args.item);
    if (charges == nullptr || charges->amount <= 0) {
        return false;
    }
    registry.Storage<component::IntentToCastSpell>().Add(
        ownership->owner,
        {.spell_id = magic->spell_id, .costs_mana = false, .direction = std::nullopt});

    if (charges->amount <= 1) {
        Unequip(registry, args.item);
        registry.DestroyEntity(args.item);
    } else {
        registry.GetRef<ChargesComponent>(args.item).amount -= 1;
    }
    return true;
}
} // namespace game_item
