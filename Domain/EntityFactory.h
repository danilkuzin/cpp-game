#pragma once
#include "EntityStorage.h"
#include "GameDatabase.h"
#include "GameItem.h"
#include "Hero.h"
#include "Location.h"
#include "Monster.h"
#include "Registry.h"
#include <optional>
#include <string>

namespace entity {
struct HeroGenerationParameters {
    std::string name;
    hero::Race race = hero::Race::Human;
    hero::HClass hclass = hero::HClass::Warrior;
};

entity::EntityId CreateHero(domain::Registry &registry, const HeroGenerationParameters &hparams,
                            Location loc);
entity::EntityId CreateMonster(domain::Registry &registry, const monster::MonsterPrefab &prefab,
                               Location loc);
entity::EntityId CreateItem(domain::Registry &registry, const game_item::ItemTemplate &prefab,
                            std::optional<Location> loc,
                            std::optional<game_item::OwnershipComponent> owner);

entity::EntityId CreateCurrency(domain::Registry &registry, const database::GameDatabase &db,
                                Location loc, int amount);

entity::EntityId CreateTrap(domain::Registry &registry, int difficulty, Location loc);
entity::EntityId CreateHealingSource(domain::Registry &registry, int amount, Location loc);

std::string GetStartingItemName(hero::HClass hclass);

void GiveStartingKit(domain::Registry &registry, const database::GameDatabase &db,
                     entity::EntityId owner, hero::HClass hclass);
void GiveMonsterKit(domain::Registry &registry, const database::GameDatabase &db,
                    entity::EntityId owner);

void GiveShopkeeperStock(domain::Registry &registry, const database::GameDatabase &db,
                         entity::EntityId shopkeeper);
} // namespace entity
