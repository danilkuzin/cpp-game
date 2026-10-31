#include "GameDatabase.h"
#include "GameItem.h"
#include "Monster.h"
#include "Spell.h"
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace database {
void GameDatabase::AddItem(const game_item::ItemTemplate &item) {
    item_prototypes_.emplace(item.id, item);
}

void GameDatabase::AddMonster(const monster::MonsterPrefab &monster) {
    monster_prototypes_.emplace(monster.id, monster);
}

void GameDatabase::AddSpell(const spell::SpellData &spell) { spells_.emplace(spell.id, spell); }

namespace {
template <typename T>
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
const T &LookUp(const std::unordered_map<PrototypeId, T> &prototypes, const PrototypeId &id,
                const char *kind) {
    auto it = prototypes.find(id);
    if (it == prototypes.end()) {
        throw std::out_of_range(std::string("unknown ") + kind + " prototype id: '" + id + "'");
    }
    return it->second;
}
} // namespace

const spell::SpellData &GameDatabase::GetSpell(const PrototypeId &id) const {
    return LookUp(spells_, id, "spell");
}

const game_item::ItemTemplate &GameDatabase::GetItem(const PrototypeId &id) const {
    return LookUp(item_prototypes_, id, "item");
}

const std::unordered_map<PrototypeId, monster::MonsterPrefab> &GameDatabase::GetMonsters() const {
    return monster_prototypes_;
}

const std::unordered_map<PrototypeId, game_item::ItemTemplate> &GameDatabase::GetItems() const {
    return item_prototypes_;
}

const std::unordered_map<PrototypeId, spell::SpellData> &GameDatabase::GetSpells() const {
    return spells_;
}
} // namespace database
