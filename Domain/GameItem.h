#pragma once
#include "EntityStorage.h"
#include "Roll.h"
#include "Spell.h"
#include <bit>
#include <cstdint>
#include <optional>
#include <string>

namespace game_item {

using ItemTemplateId = std::string;

struct MeleeWeaponComponent {
    roll::Dices damage;
};

struct ArmorComponent {
    int defence = 0;
};

struct AmmoComponent {
    int amount = 0;
};

struct LoadedAmmoComponent {
    int amount = 0;
};

struct RangedWeaponComponent {
    roll::Dices damage;
    int max_ammo = 0;
    int range = 0;
};

struct MagicComponent {
    spell::SpellId spell_id;
    int max_charges = 0;
};

struct ChargesComponent {
    int amount = 0;
};

enum class EquipSlot : std::uint8_t {
    None = 0,
    Head = 1 << 0,
    Chest = 1 << 1,
    MainHand = 1 << 2,
    OffHand = 1 << 3,
    RingLeft = 1 << 4,
    RingRight = 1 << 5,

    AnyHand = MainHand | OffHand,
    AnyRing = RingLeft | RingRight
};

inline bool CanFitInSlot(EquipSlot item_allowed, EquipSlot target_slot) {
    return (static_cast<uint32_t>(item_allowed) & static_cast<uint32_t>(target_slot)) != 0;
}

inline bool IsConcreteSlot(EquipSlot slot) {
    return std::has_single_bit(static_cast<uint32_t>(slot));
}

struct EquippableComponent {
    EquipSlot allowed_slots = EquipSlot::None;
};

struct EquippedComponent {
    EquipSlot slot = EquipSlot::None;
};

struct CurrencyComponent {
    int amount = 0;
};

struct ValueComponent {
    int base_price = 0;
};

struct SizeComponent {
    int size = 0;
};

struct OwnershipComponent {
    entity::EntityId owner = 0;
};

inline const ItemTemplateId &CurrencyItemId() {
    static const ItemTemplateId kId = "copper";
    return kId;
}

struct ItemTemplate {
    std::string id;
    std::string name;

    std::optional<MeleeWeaponComponent> melee;
    std::optional<RangedWeaponComponent> ranged;
    std::optional<ArmorComponent> armor;
    std::optional<AmmoComponent> ammo;
    std::optional<MagicComponent> magic;
    std::optional<EquippableComponent> equippable;
    std::optional<CurrencyComponent> currency;
    std::optional<ValueComponent> value;

    int size = 0;
    std::string sprite_name;
};

} // namespace game_item
