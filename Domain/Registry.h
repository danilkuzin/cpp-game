#pragma once

#include "ComponentStorage.h"
#include "Components.h"
#include "EntityStorage.h"
#include "GameItem.h"
#include "Hero.h"
#include "Location.h"
#include "Map.h"
#include "Terrain.h"
#include <cassert>
#include <tuple>
#include <vector>

namespace domain {

class Registry {
public:
    entity::EntityStorage entities;

    template <typename T> [[nodiscard]] component::ComponentStorage<T> &Storage() {
        return std::get<component::ComponentStorage<T>>(storages_);
    }

    template <typename T> [[nodiscard]] const component::ComponentStorage<T> &Storage() const {
        return std::get<component::ComponentStorage<T>>(storages_);
    }

    std::vector<map::Map> maps;
    map::TerrainRegistry terrain_registry;

    template <typename... Cs>
    [[nodiscard]] bool HasAll([[maybe_unused]] entity::EntityId id) const {
        return (Storage<Cs>().Has(id) && ...);
    }

    template <typename First, typename... Rest>
    [[nodiscard]] std::vector<entity::EntityId> View() const {
        std::vector<entity::EntityId> out;
        for (const auto &kv : Storage<First>()) {
            if (HasAll<Rest...>(kv.first)) {
                out.push_back(kv.first);
            }
        }
        return out;
    }

    template <typename T> [[nodiscard]] T &GetRef(entity::EntityId id) {
        auto *p = Storage<T>().Get(id);
        assert(p != nullptr);
        return *p;
    }

    template <typename T> [[nodiscard]] const T &GetRef(entity::EntityId id) const {
        const auto *p = Storage<T>().Get(id);
        assert(p != nullptr);
        return *p;
    }

    Registry() = default;
    ~Registry() = default;
    Registry(const Registry &) = delete;
    Registry &operator=(const Registry &) = delete;
    Registry(Registry &&) = default;
    Registry &operator=(Registry &&) = default;

    void PlaceEntity(entity::EntityId id, Location loc) { Storage<Location>().Add(id, loc); }

    void ClearLocation(entity::EntityId id) { Storage<Location>().Remove(id); }

    void DestroyEntity(entity::EntityId id) {
        entities.Remove(id);
        ForEachStorage([id](auto &storage) { storage.Remove(id); });
    }

    template <typename F> void ForEachStorage(F &&fn) {
        std::apply([&fn](auto &...storage) { (fn(storage), ...); }, storages_);
    }

    template <typename F> void ForEachStorage(F &&fn) const {
        std::apply([&fn](const auto &...storage) { (fn(storage), ...); }, storages_);
    }

private:
    std::tuple<component::ComponentStorage<component::NameComponent>,
               component::ComponentStorage<component::VisionComponent>,
               component::ComponentStorage<Location>,
               component::ComponentStorage<component::IntentToMove>,
               component::ComponentStorage<component::IntentToShoot>,
               component::ComponentStorage<component::PlayerTag>,
               component::ComponentStorage<component::MonsterTag>,
               component::ComponentStorage<component::RenderComponent>,
               component::ComponentStorage<component::IntentToMelee>,
               component::ComponentStorage<component::HealthComponent>,
               component::ComponentStorage<component::ManaComponent>,
               component::ComponentStorage<component::IntentToTakeDamage>,
               component::ComponentStorage<component::IntentToHeal>,
               component::ComponentStorage<component::IntentToCastSpell>,
               component::ComponentStorage<component::AttributesComponent>,
               component::ComponentStorage<game_item::MeleeWeaponComponent>,
               component::ComponentStorage<game_item::RangedWeaponComponent>,
               component::ComponentStorage<game_item::ArmorComponent>,
               component::ComponentStorage<game_item::AmmoComponent>,
               component::ComponentStorage<game_item::LoadedAmmoComponent>,
               component::ComponentStorage<game_item::EquippableComponent>,
               component::ComponentStorage<game_item::EquippedComponent>,
               component::ComponentStorage<game_item::CurrencyComponent>,
               component::ComponentStorage<component::PurseComponent>,
               component::ComponentStorage<game_item::ValueComponent>,
               component::ComponentStorage<game_item::SizeComponent>,
               component::ComponentStorage<game_item::OwnershipComponent>,
               component::ComponentStorage<game_item::MagicComponent>,
               component::ComponentStorage<game_item::ChargesComponent>,
               component::ComponentStorage<component::ItemTag>,
               component::ComponentStorage<component::ShopkeeperTag>,
               component::ComponentStorage<hero::HeroSkills>,
               component::ComponentStorage<hero::HeroExperience>,
               component::ComponentStorage<map::TrapComponent>,
               component::ComponentStorage<map::HealingSourceComponent>>
        storages_;
};
} // namespace domain
