#pragma once

#include <array>
#include <string>
#include <string_view>
#include <unordered_map>

// A function-local static defers construction to first call,
// where a throw is catchable exception.
namespace texts {
inline const std::string &StrHeroHp() {
    static const std::string kValue = " HP/MaxHP: ";
    return kValue;
}

inline const std::string &StrHeroMana() {
    static const std::string kValue = " Mana/MaxMana: ";
    return kValue;
}

inline const std::string &StrHeroXy() {
    static const std::string kValue = " Coords: ";
    return kValue;
}

inline const std::string &StrHeroMoney() {
    static const std::string kValue = " Copper: ";
    return kValue;
}

inline const std::unordered_map<std::string, std::string> &NpcNames() {
    static const std::unordered_map<std::string, std::string> kValue = {
        {"STR_MONSTER1", "Rat"},          {"STR_MONSTER2", "Wolf"},
        {"STR_MONSTER3", "Giant Spider"}, {"STR_MONSTER4", "Skeleton"},
        {"STR_MONSTER5", "Orc"},          {"STR_MONSTER6", "Troll"},
        {"STR_MONSTER7", "Giant Snake"},  {"STR_SHOPKEEPER", "Shopkeeper"},
    };
    return kValue;
}

inline const std::string &StrShop() {
    static const std::string kValue = "Shop";
    return kValue;
}

inline const std::string &StrTrap() {
    static const std::string kValue = "Trap activated";
    return kValue;
}

inline const std::string &StrTrapOk() {
    static const std::string kValue = "Trap found and deactivated";
    return kValue;
}

inline const std::string &StrHeroDied() {
    static const std::string kValue = "Hero died";
    return kValue;
}

inline const std::string &StrLive() {
    static const std::string kValue = "Hero healed in life source";
    return kValue;
}

inline const std::string &StrAxe() {
    static const std::string kValue = "Axe";
    return kValue;
}

inline const std::string &StrSword() {
    static const std::string kValue = "Sword";
    return kValue;
}

inline const std::string &StrHelm() {
    static const std::string kValue = "Helm";
    return kValue;
}

inline const std::string &StrBodyArmor() {
    static const std::string kValue = "Bodyarmor";
    return kValue;
}

inline const std::string &StrAmmo() {
    static const std::string kValue = "Arrows";
    return kValue;
}

inline const std::string &StrCross() {
    static const std::string kValue = "Bow";
    return kValue;
}

inline const std::string &StrMagicItem() {
    static const std::string kValue = "Magic Item";
    return kValue;
}

inline const std::string &StrHeroItems() {
    static const std::string kValue = "Items";
    return kValue;
}

inline const std::string &StrHeroSlots() {
    static const std::string kValue = "Slots";
    return kValue;
}

inline const std::string &StrEmptyItem() {
    static const std::string kValue = "Empty";
    return kValue;
}

inline const std::string &StrItemPickedUp() {
    static const std::string kValue = "Item picked up";
    return kValue;
}

inline const std::string &StrItemDropped() {
    static const std::string kValue = "Item dropped";
    return kValue;
}

inline const std::string &StrItemUsed() {
    static const std::string kValue = "Item used";
    return kValue;
}

inline const std::string &StrFullInventory() {
    static const std::string kValue = "Full inventory";
    return kValue;
}

inline const std::string &StrUnknownName() {
    static const std::string kValue = "Unknown";
    return kValue;
}

inline const std::array<std::string, 6> &SlotName() {
    static const std::array<std::string, 6> kValue = {"Body",    "Hands", "Helm",
                                                      "Offhand", "RingL", "RingR"};
    return kValue;
}

inline const std::string &LogLabel() {
    static const std::string kValue = "Log";
    return kValue;
}

inline const std::string &StrBadAttack() {
    static const std::string kValue = "missed";
    return kValue;
}

inline const std::string &StrNoneWeapons() {
    static const std::string kValue = "no weapons";
    return kValue;
}

inline const std::string &StrNoAmmo() {
    static const std::string kValue = "no ammo";
    return kValue;
}

inline const std::string &StrNoMana() {
    static const std::string kValue = "no mana";
    return kValue;
}

inline const std::string &StrBlocked() {
    static const std::string kValue = "path blocked";
    return kValue;
}

inline const std::string &StrHealed() {
    static const std::string kValue = "healed for ";
    return kValue;
}

inline const std::string &StrBigSkin() {
    static const std::string kValue = "big skin";
    return kValue;
}

inline const std::string &StrAttack() {
    static const std::string kValue = "attacked for ";
    return kValue;
}

inline const std::string &StrMonKill() {
    static const std::string kValue = "slained ";
    return kValue;
}

inline const std::string &StrMonStop() {
    static const std::string kValue = "avoided ";
    return kValue;
}

inline const std::string &StrMonDef() {
    static const std::string kValue = "defended";
    return kValue;
}

inline const std::string &StrMonAttack() {
    static const std::string kValue = "attacked by";
    return kValue;
}

inline const std::string &StrCantSell() {
    static const std::string kValue = "can't sell that";
    return kValue;
}

inline const std::string &StrCantBuy() {
    static const std::string kValue = "can't buy that";
    return kValue;
}

inline const std::string &StrCantEquip() {
    static const std::string kValue = "can't equip that";
    return kValue;
}

inline const std::string &StrCantDrop() {
    static const std::string kValue = "can't drop that";
    return kValue;
}

inline const std::string &StrCantUse() {
    static const std::string kValue = "can't use that";
    return kValue;
}

inline const std::string &StrGameOver() {
    static const std::string kValue = "Game Over";
    return kValue;
}

inline const std::string &StrGameOverPrompt() {
    static const std::string kValue = "Press any key to quit";
    return kValue;
}

inline const std::string &StrGameOverLevel() {
    static const std::string kValue = "Level: ";
    return kValue;
}

inline const std::string &StrGameOverMonstersSlain() {
    static const std::string kValue = "Monsters slain: ";
    return kValue;
}

inline const std::string &StrGameOverEquipment() {
    static const std::string kValue = "Equipment: ";
    return kValue;
}

inline const std::string &StrGameOverNoEquipment() {
    static const std::string kValue = "(nothing)";
    return kValue;
}

// These need to be compile-time
constexpr std::array<std::string_view, 4> RaceNames() {
    return {"Human", "Elf", "Dwarf", "Hobbit"};
}

constexpr std::array<std::string_view, 3> ClassNames() { return {"Warrior", "Ranger", "Mage"}; }

inline const std::array<std::string, 6> &CharNames() {
    static const std::array<std::string, 6> kValue = {"STR", "DEX", "CON", "IQ", "WIS", "CHA"};
    return kValue;
}

inline const std::array<std::string, 5> &SkillNames() {
    static const std::array<std::string, 5> kValue = {"HandWeapon", "TrapSearch", "Defence",
                                                      "RangedWeapon", "Spellcasting"};
    return kValue;
}

inline const std::array<std::string, 4> &MagicItemsNames() {
    static const std::array<std::string, 4> kValue = {"Firestorm Crystal", "Healing Potion",
                                                      "Firestorm Staff", "Healing Staff"};
    return kValue;
}
} // namespace texts
