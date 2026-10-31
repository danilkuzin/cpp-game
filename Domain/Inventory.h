#pragma once

#include "EntityStorage.h"
#include "Event.h"
#include "GameItem.h"
#include "Registry.h"
#include <optional>
#include <span>
#include <vector>

namespace game_item {

constexpr int kMaxCarrySize = 12;

std::vector<entity::EntityId> OwnedItems(const domain::Registry &registry, entity::EntityId actor);
int CarriedSize(const domain::Registry &registry, entity::EntityId actor);
std::vector<entity::EntityId> EquippedItems(const domain::Registry &registry,
                                            entity::EntityId actor);
std::vector<entity::EntityId> UnequippedItems(const domain::Registry &registry,
                                              entity::EntityId actor);
std::optional<entity::EntityId> EquippedInSlot(const domain::Registry &registry,
                                               entity::EntityId actor, EquipSlot slot);
int Money(const domain::Registry &registry, entity::EntityId actor);
void AddMoney(domain::Registry &registry, entity::EntityId actor, int amount);

struct TransferItemArgs {
    entity::EntityId item = 0;
    entity::EntityId actor = 0;
};

bool TransferItem(domain::Registry &registry, TransferItemArgs args);
bool PickUp(domain::Registry &registry, TransferItemArgs args);
event::Result PickUpFromTile(domain::Registry &registry, entity::EntityId actor);
bool Drop(domain::Registry &registry, TransferItemArgs args);

bool Equip(domain::Registry &registry, TransferItemArgs args, EquipSlot target_slot);

bool EquipToFirstFittingSlot(domain::Registry &registry, TransferItemArgs args,
                             std::span<const EquipSlot> candidate_slots);
void Unequip(domain::Registry &registry, entity::EntityId item);
bool UseItem(domain::Registry &registry, TransferItemArgs args);

} // namespace game_item
