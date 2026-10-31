#pragma once
#include "GameItem.h"
#include "Monster.h"
#include "Spell.h"
#include <string>
#include <unordered_map>

using PrototypeId = std::string;

namespace database {
class GameDatabase {
public:
    void AddItem(const game_item::ItemTemplate &item);
    void AddMonster(const monster::MonsterPrefab &monster);
    void AddSpell(const spell::SpellData &spell);

    [[nodiscard]] const spell::SpellData &GetSpell(const PrototypeId &id) const;
    [[nodiscard]] const game_item::ItemTemplate &GetItem(const PrototypeId &id) const;
    [[nodiscard]] const std::unordered_map<PrototypeId, monster::MonsterPrefab> &
    GetMonsters() const;
    [[nodiscard]] const std::unordered_map<PrototypeId, game_item::ItemTemplate> &GetItems() const;
    [[nodiscard]] const std::unordered_map<PrototypeId, spell::SpellData> &GetSpells() const;

private:
    std::unordered_map<PrototypeId, game_item::ItemTemplate> item_prototypes_;
    std::unordered_map<PrototypeId, monster::MonsterPrefab> monster_prototypes_;
    std::unordered_map<PrototypeId, spell::SpellData> spells_;
};
} // namespace database
