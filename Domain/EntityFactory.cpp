#include "EntityFactory.h"
#include "Components.h"
#include "EntityStorage.h"
#include "GameDatabase.h"
#include "GameItem.h"
#include "Hero.h"
#include "Inventory.h"
#include "Location.h"
#include "Map.h"
#include "Monster.h"
#include "Registry.h"
#include "Roll.h"
#include "Tables.h"
#include <array>
#include <cassert>
#include <cstddef>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace entity {

entity::EntityId CreateHero(domain::Registry &registry, const HeroGenerationParameters &hparams,
                            Location loc) {
    auto entity_id = registry.entities.CreateEntity();
    registry.Storage<component::NameComponent>().Add(entity_id, {.name = hparams.name});
    registry.Storage<component::VisionComponent>().Add(entity_id,
                                                       {.view_zone = hero::kVisionRadius});
    registry.PlaceEntity(entity_id, loc);
    auto hclass_idx = std::to_underlying(hparams.hclass);
    registry.Storage<component::HealthComponent>().Add(
        entity_id,
        {.current = hero::kHpLevelTable[hclass_idx][0], .max = hero::kHpLevelTable[hclass_idx][0]});
    registry.Storage<component::ManaComponent>().Add(
        entity_id, {.current = hero::kManaLevelTable[hclass_idx][0],
                    .max = hero::kManaLevelTable[hclass_idx][0]});
    registry.Storage<component::PlayerTag>().Add(entity_id, {});
    registry.Storage<component::RenderComponent>().Add(entity_id,
                                                       {.tile = "hero.bmp", .z_index = 3});
    registry.Storage<component::PurseComponent>().Add(entity_id, {.money = 0});

    auto race_idx = std::to_underlying(hparams.race);
    std::array<int, hero::kMaxChars> chars{};
    for (int i = 0; i < hero::kMaxChars; i++) {
        auto min = hero::kRaceTable[race_idx][i].first;
        auto increment = hero::kRaceTable[race_idx][i].second;
        chars[i] = min + roll::Index(increment + 1);
    }

    registry.Storage<component::AttributesComponent>().Add(entity_id, {.chars = chars});

    std::array<int, static_cast<std::size_t>(hero::Skill::MaxSkills) + 1> skills{};
    for (size_t si = 0; si < skills.size(); si++) {
        auto min = hero::kClassSkillTable[si][hclass_idx].first;
        auto increment = hero::kClassSkillTable[si][hclass_idx].second;
        skills[si] = min + roll::Index(increment + 1);
    }
    registry.Storage<hero::HeroSkills>().Add(entity_id, {.skills = skills});

    registry.Storage<hero::HeroExperience>().Add(entity_id,
                                                 {.xp = 0, .level = 1, .hclass = hparams.hclass});

    return entity_id;
}

entity::EntityId CreateMonster(domain::Registry &registry, const monster::MonsterPrefab &prefab,
                               Location loc) {
    auto entity_id = registry.entities.CreateEntity();
    registry.Storage<component::NameComponent>().Add(entity_id, {prefab.name});
    registry.Storage<component::VisionComponent>().Add(entity_id, {prefab.view_zone});
    registry.PlaceEntity(entity_id, loc);
    registry.Storage<component::HealthComponent>().Add(
        entity_id, {.current = prefab.max_hp, .max = prefab.max_hp});
    registry.Storage<component::AttributesComponent>().Add(entity_id, {.chars = prefab.chars});

    registry.Storage<component::MonsterTag>().Add(entity_id, {});
    registry.Storage<component::RenderComponent>().Add(entity_id,
                                                       {.tile = prefab.tile, .z_index = 2});
    if (prefab.shopkeeper) {
        registry.Storage<component::ShopkeeperTag>().Add(entity_id, {});
    }
    return entity_id;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
entity::EntityId CreateItem(domain::Registry &registry, const game_item::ItemTemplate &prefab,
                            std::optional<Location> loc,
                            std::optional<game_item::OwnershipComponent> owner) {
    assert(((loc == std::nullopt) || (owner == std::nullopt)) &&
           ((loc.has_value()) || (owner.has_value())));
    if (prefab.ranged.has_value() && prefab.ammo.has_value()) {
        throw std::invalid_argument("item template '" + prefab.id +
                                    "' sets both ranged and ammo; an item is a weapon or "
                                    "ammunition, not both");
    }
    auto entity_id = registry.entities.CreateEntity();

    registry.Storage<component::NameComponent>().Add(entity_id, {prefab.name});
    registry.Storage<game_item::SizeComponent>().Add(entity_id, {prefab.size});

    registry.Storage<component::RenderComponent>().Add(entity_id,
                                                       {.tile = prefab.sprite_name, .z_index = 1});
    if (prefab.melee.has_value()) {
        registry.Storage<game_item::MeleeWeaponComponent>().Add(entity_id, prefab.melee.value());
    }
    if (prefab.ranged.has_value()) {
        registry.Storage<game_item::RangedWeaponComponent>().Add(entity_id, prefab.ranged.value());
        registry.Storage<game_item::LoadedAmmoComponent>().Add(entity_id,
                                                               {.amount = prefab.ranged->max_ammo});
    }
    if (prefab.armor.has_value()) {
        registry.Storage<game_item::ArmorComponent>().Add(entity_id, prefab.armor.value());
    }
    if (prefab.ammo.has_value()) {
        registry.Storage<game_item::AmmoComponent>().Add(entity_id, prefab.ammo.value());
    }
    if (prefab.equippable.has_value()) {
        registry.Storage<game_item::EquippableComponent>().Add(entity_id,
                                                               prefab.equippable.value());
    }
    if (prefab.magic.has_value()) {
        registry.Storage<game_item::MagicComponent>().Add(entity_id, prefab.magic.value());
        registry.Storage<game_item::ChargesComponent>().Add(entity_id,
                                                            {.amount = prefab.magic->max_charges});
    }
    if (prefab.currency.has_value()) {
        registry.Storage<game_item::CurrencyComponent>().Add(entity_id, prefab.currency.value());
    }
    if (prefab.value.has_value()) {
        registry.Storage<game_item::ValueComponent>().Add(entity_id, prefab.value.value());
    }
    if (loc.has_value()) {
        registry.PlaceEntity(entity_id, loc.value());
    }
    if (owner.has_value()) {
        registry.Storage<game_item::OwnershipComponent>().Add(entity_id, owner.value());
    }
    registry.Storage<component::ItemTag>().Add(entity_id, {});

    return entity_id;
}

entity::EntityId CreateCurrency(domain::Registry &registry, const database::GameDatabase &db,
                                Location loc, int amount) {
    auto prefab = db.GetItem(game_item::CurrencyItemId());
    prefab.currency = game_item::CurrencyComponent{.amount = amount};
    return CreateItem(registry, prefab, loc, std::nullopt);
}

entity::EntityId CreateTrap(domain::Registry &registry, int difficulty, Location loc) {
    auto entity_id = registry.entities.CreateEntity();

    registry.Storage<map::TrapComponent>().Add(entity_id,
                                               {.active = true, .difficulty = difficulty});
    registry.PlaceEntity(entity_id, loc);

    return entity_id;
}

entity::EntityId CreateHealingSource(domain::Registry &registry, int amount, Location loc) {
    auto entity_id = registry.entities.CreateEntity();

    registry.Storage<map::HealingSourceComponent>().Add(entity_id, {.amount = amount});
    registry.PlaceEntity(entity_id, loc);
    return entity_id;
}

std::string GetStartingItemName(hero::HClass hclass) {
    if (hclass == hero::HClass::Ranger) {
        return "crossbow_standard";
    }
    return "axe_standard";
}

void GiveStartingKit(domain::Registry &registry, const database::GameDatabase &db,
                     entity::EntityId owner, hero::HClass hclass) {
    const auto &items = db.GetItems();
    const auto weapon_template_id = GetStartingItemName(hclass);

    auto weapon = items.find(weapon_template_id);
    if (weapon == items.end()) {
        return;
    }

    auto weapon_id = CreateItem(registry, weapon->second, std::nullopt,
                                game_item::OwnershipComponent{.owner = owner});
    game_item::Equip(registry, {.item = weapon_id, .actor = owner}, game_item::EquipSlot::MainHand);
}

void GiveMonsterKit(domain::Registry &registry, const database::GameDatabase &db,
                    entity::EntityId owner) {
    registry.Storage<component::PurseComponent>().Add(
        owner, {.money = roll::Dice({.count = 2, .sides = 8})});
    std::vector<game_item::ItemTemplate> cur_prefabs;
    for (const auto &kv : db.GetItems()) {
        cur_prefabs.push_back(kv.second);
    }

    if (!cur_prefabs.empty()) {
        auto item_type = roll::Index(static_cast<int>(cur_prefabs.size()));
        CreateItem(registry, cur_prefabs[item_type], std::nullopt,
                   game_item::OwnershipComponent{.owner = owner});
    }
}

namespace {
constexpr int kShopkeeperStartingMoney = 2000;
} // namespace

void GiveShopkeeperStock(domain::Registry &registry, const database::GameDatabase &db,
                         entity::EntityId shopkeeper) {
    registry.Storage<component::PurseComponent>().Add(shopkeeper,
                                                      {.money = kShopkeeperStartingMoney});
    for (const auto &[item_id, item_template] : db.GetItems()) {
        if (!item_template.value.has_value()) {
            continue;
        }
        CreateItem(registry, item_template, std::nullopt,
                   game_item::OwnershipComponent{.owner = shopkeeper});
    }
}

} // namespace entity
