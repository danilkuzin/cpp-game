#include "DataLoader.h"
#include "Config.h"
#include "GameDatabase.h"
#include "GameItem.h"
#include "Hero.h"
#include "Monster.h"
#include "Spell.h"
#include "Texts.h"
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <unordered_map>

namespace dataloader {
namespace {
game_item::MeleeWeaponComponent ParseMelee(const nlohmann::json &j) {
    return {.damage = {.count = j["damage"].at("dice"), .sides = j["damage"].at("sides")}};
}

game_item::ArmorComponent ParseArmor(const nlohmann::json &j) { return {j.at("defence")}; }

game_item::MagicComponent ParseMagic(const nlohmann::json &j) {
    return {.spell_id = j.at("spell_id"), .max_charges = j.at("max_charges")};
}

game_item::RangedWeaponComponent ParseRanged(const nlohmann::json &j) {
    return {.damage = {.count = j["damage"].at("dice"), .sides = j["damage"].at("sides")},
            .max_ammo = j.at("max_ammo"),
            .range = j.at("range")};
}

game_item::AmmoComponent ParseAmmo(const nlohmann::json &j) { return {j.at("num_per_bag")}; }

game_item::ValueComponent ParseValue(const nlohmann::json &j) { return {j.at("base_price")}; }

inline game_item::EquipSlot operator|(game_item::EquipSlot a, game_item::EquipSlot b) {
    return static_cast<game_item::EquipSlot>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline game_item::EquipSlot &operator|=(game_item::EquipSlot &a, game_item::EquipSlot b) {
    return a = a | b;
}

game_item::EquippableComponent ParseEquippable(const nlohmann::json &j) {
    static const std::unordered_map<std::string, game_item::EquipSlot> kSlotMap = {
        {"Head", game_item::EquipSlot::Head},
        {"Chest", game_item::EquipSlot::Chest},
        {"MainHand", game_item::EquipSlot::MainHand},
        {"OffHand", game_item::EquipSlot::OffHand},
        {"RingLeft", game_item::EquipSlot::RingLeft},
        {"RingRight", game_item::EquipSlot::RingRight},
        {"AnyHand", game_item::EquipSlot::AnyHand},
        {"AnyRing", game_item::EquipSlot::AnyRing}};

    game_item::EquippableComponent comp{.allowed_slots = game_item::EquipSlot::None};

    if (j.contains("slots") && j["slots"].is_array()) {
        for (const auto &slot_elem : j["slots"]) {
            const std::string slot_name = slot_elem.get<std::string>();

            auto it = kSlotMap.find(slot_name);
            if (it != kSlotMap.end()) {
                comp.allowed_slots |= it->second;
            }
        }
    }

    return comp;
}

game_item::CurrencyComponent ParseCurrency([[maybe_unused]] const nlohmann::json &j) { return {}; }
} // namespace

void DataLoader::LoadMonsters(const std::string &path, database::GameDatabase &db) {
    std::ifstream file(path);
    nlohmann::json data = nlohmann::json::parse(file);

    for (const auto &monster_template_json : data["npcs"]) {
        monster::MonsterPrefab monster_template;
        monster_template.id = monster_template_json["id"];
        monster_template.name = texts::NpcNames().at(monster_template_json["name"]);
        monster_template.max_hp = monster_template_json["max_hp"];
        monster_template.map_level = monster_template_json["map_level"];
        if (monster_template_json.contains("attributes")) {
            monster_template.chars[hero::kChrStr] = monster_template_json["attributes"]["str"];
            monster_template.chars[hero::kChrDex] = monster_template_json["attributes"]["dex"];
            monster_template.chars[hero::kChrCon] =
                monster_template_json["attributes"].value("con", 0);
            monster_template.chars[hero::kChrIq] = monster_template_json["attributes"]["iq"];
            monster_template.chars[hero::kChrWis] = monster_template_json["attributes"]["wis"];
            monster_template.chars[hero::kChrCha] = monster_template_json["attributes"]["cha"];
        }
        monster_template.view_zone = monster_template_json["view_zone"];
        monster_template.tile = monster_template_json["tile"];

        if (monster_template_json.contains("shopkeeper")) {
            monster_template.shopkeeper = true;
        }
        db.AddMonster(monster_template);
    }
}

void DataLoader::LoadItems(const std::string &path, database::GameDatabase &db) {
    std::ifstream file(path);
    nlohmann::json data = nlohmann::json::parse(file);

    for (const auto &item_json : data["items"]) {
        game_item::ItemTemplate item;
        item.id = item_json.at("id");
        item.name = item_json.at("name");
        item.size = item_json.at("size");
        item.sprite_name = item_json.at("sprite_name");

        if (item_json.contains("melee")) {
            item.melee = ParseMelee(item_json["melee"]);
        }
        if (item_json.contains("armor")) {
            item.armor = ParseArmor(item_json["armor"]);
        }
        if (item_json.contains("ammo")) {
            item.ammo = ParseAmmo(item_json["ammo"]);
        }
        if (item_json.contains("ranged")) {
            item.ranged = ParseRanged(item_json["ranged"]);
        }
        if (item_json.contains("magic")) {
            item.magic = ParseMagic(item_json["magic"]);
        }
        if (item_json.contains("equippable")) {
            item.equippable = ParseEquippable(item_json["equippable"]);
        }
        if (item_json.contains("currency")) {
            item.currency = ParseCurrency(item_json["currency"]);
        }
        if (item_json.contains("value")) {
            item.value = ParseValue(item_json["value"]);
        }

        db.AddItem(item);
    }
}

void DataLoader::LoadSpells(const std::string &path, database::GameDatabase &db) {
    std::ifstream file(path);
    nlohmann::json data = nlohmann::json::parse(file);

    for (const auto &spell_json : data["spells"]) {
        spell::SpellData spell;
        spell.name = spell_json["name"];
        spell.id = spell_json["id"];

        if (spell_json.contains("mana_cost")) {
            spell.mana_cost = spell::ManaCostComponent{.amount = spell_json["mana_cost"]};
        }
        if (spell_json.contains("damage")) {
            spell.damage = spell::DamageComponent{.amount = spell_json["damage"]["amount"]};
        }
        if (spell_json.contains("healing")) {
            spell.healing = spell::HealingComponent{.amount = spell_json["healing"]["amount"]};
        }
        db.AddSpell(spell);
    }
}

database::GameDatabase DataLoader::LoadDatabase(const std::string &path) {
    database::GameDatabase db;
    LoadMonsters(path + "/npc.json", db);
    LoadItems(path + "/items.json", db);
    LoadSpells(path + "/spells.json", db);
    return db;
}

domain::Config DataLoader::LoadConfig(const std::string &path) {
    std::ifstream file(path);

    nlohmann::json data = nlohmann::json::parse(file);

    std::optional<unsigned int> seed;
    if (data.contains("seed")) {
        seed = data["seed"].get<unsigned int>();
    }

    return domain::Config{.map_config = {.width = data["map"]["width"],
                                         .height = data["map"]["height"],
                                         .max_level = data["map"]["max_level"],
                                         .border_size = data["map"]["border_size"]},
                          .num_monsters = data["monsters"]["start_num"],
                          .seed = seed};
}

domain::ViewConfig DataLoader::LoadViewConfig(const std::string &path) {
    std::ifstream file(path);

    const nlohmann::json data = nlohmann::json::parse(file);

    return domain::ViewConfig{.camera_width = data["camera"]["width"],
                              .camera_height = data["camera"]["height"],
                              .scroll_delta = data["camera"]["scroll_delta"]};
}
} // namespace dataloader
