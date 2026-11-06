#pragma once

#include "Components.h"
#include "GameItem.h"
#include "Hero.h"
#include "Location.h"
#include "RectInt.h"
#include "Registry.h"
#include "Spell.h"
#include "Vector2Int.h"
#include <gtest/gtest.h>
#include <string>
#include <utility>

inline entity::EntityId MakeEntityAt(domain::Registry &registry, Location loc) {
    auto id = registry.entities.CreateEntity();
    registry.Storage<Location>().Add(id, loc);
    return id;
}

inline entity::EntityId MakeCreatureAt(domain::Registry &registry, Location loc, int hp,
                                       bool is_monster) {
    auto id = MakeEntityAt(registry, loc);
    registry.Storage<component::HealthComponent>().Add(id, {.current = hp, .max = 10});
    registry.Storage<component::AttributesComponent>().Add(id, {});
    if (is_monster) {
        registry.Storage<component::MonsterTag>().Add(id, {});
    } else {
        registry.Storage<component::PlayerTag>().Add(id, {});
    }
    return id;
}

inline entity::EntityId MakeGroundItemAt(domain::Registry &registry, Location loc, int size) {
    auto id = MakeEntityAt(registry, loc);
    registry.Storage<game_item::SizeComponent>().Add(id, {.size = size});
    return id;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
inline entity::EntityId MakeCarriedItem(domain::Registry &registry, entity::EntityId owner,
                                        int size) {
    auto id = registry.entities.CreateEntity();
    registry.Storage<game_item::OwnershipComponent>().Add(id, {.owner = owner});
    registry.Storage<game_item::SizeComponent>().Add(id, {.size = size});
    return id;
}

inline entity::EntityId MakeCarriedEquippableItem(domain::Registry &registry,
                                                  entity::EntityId owner,
                                                  game_item::EquipSlot allowed_slots) {
    auto id = MakeCarriedItem(registry, owner, 1);
    registry.Storage<game_item::EquippableComponent>().Add(id, {.allowed_slots = allowed_slots});
    return id;
}

inline entity::EntityId MakeCarriedMagicItem(domain::Registry &registry, entity::EntityId owner,
                                             spell::SpellId spell_id, int charges) {
    auto id = MakeCarriedItem(registry, owner, 1);
    registry.Storage<game_item::MagicComponent>().Add(
        id, {.spell_id = std::move(spell_id), .max_charges = charges});
    registry.Storage<game_item::ChargesComponent>().Add(id, {.amount = charges});
    return id;
}

inline entity::EntityId MakeEquippedItem(domain::Registry &registry, entity::EntityId owner,
                                         game_item::EquipSlot slot) {
    auto id = registry.entities.CreateEntity();
    registry.Storage<game_item::OwnershipComponent>().Add(id, {.owner = owner});
    registry.Storage<game_item::EquippedComponent>().Add(id, {.slot = slot});
    return id;
}

inline component::AttributesComponent WithChar(int chr_index, int value) {
    component::AttributesComponent attrs{};
    attrs.chars[chr_index] = value;
    return attrs;
}

inline component::AttributesComponent WithStr(int str) { return WithChar(hero::kChrStr, str); }

// NOLINTNEXTLINE(readability-suspicious-call-argument)
inline component::AttributesComponent WithDex(int dex) { return WithChar(hero::kChrDex, dex); }

inline component::AttributesComponent WithCha(int cha) { return WithChar(hero::kChrCha, cha); }

inline entity::EntityId MakePlayerMoverAt(domain::Registry &registry, Location loc, int hp,
                                          int max_hp) {
    auto id = registry.entities.CreateEntity();
    registry.Storage<component::HealthComponent>().Add(id, {.current = hp, .max = max_hp});
    registry.Storage<Location>().Add(id, loc);
    registry.Storage<component::PlayerTag>().Add(id, {});
    registry.Storage<component::VisionComponent>().Add(id, {.view_zone = 1});
    return id;
}

inline entity::EntityId MakeHeroAt(domain::Registry &registry, Vector2Int coord) {
    auto id = MakeCreatureAt(registry, {.coord = coord, .map_level = 0}, 10, false);
    registry.Storage<component::VisionComponent>().Add(id, {.view_zone = 1});
    registry.Storage<component::NameComponent>().Add(id, {.name = "Hero"});
    registry.Storage<hero::HeroExperience>().Add(id, {});
    registry.Storage<component::ManaComponent>().Add(id, {.current = 5, .max = 5});
    return id;
}

inline entity::EntityId MakeNamedMonsterAt(domain::Registry &registry, Vector2Int coord, int hp,
                                           const std::string &name) {
    auto id = MakeCreatureAt(registry, {.coord = coord, .map_level = 0}, hp, true);
    registry.Storage<component::NameComponent>().Add(id, {.name = name});
    return id;
}

inline std::pair<float, float> RectCenter(RectInt rect) {
    return {static_cast<float>(rect.x) + (static_cast<float>(rect.width) / 2.0F),
            static_cast<float>(rect.y) + (static_cast<float>(rect.height) / 2.0F)};
}

inline void ExpectNoIntentsSurvived(const domain::Registry &registry) {
    EXPECT_TRUE(registry.Storage<component::IntentToMove>().Empty());
    EXPECT_TRUE(registry.Storage<component::IntentToMelee>().Empty());
    EXPECT_TRUE(registry.Storage<component::IntentToShoot>().Empty());
    EXPECT_TRUE(registry.Storage<component::IntentToTakeDamage>().Empty());
    EXPECT_TRUE(registry.Storage<component::IntentToHeal>().Empty());
    EXPECT_TRUE(registry.Storage<component::IntentToCastSpell>().Empty());
}
