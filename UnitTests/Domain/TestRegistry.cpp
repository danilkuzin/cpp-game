#include "Components.h"
#include "Location.h"
#include "Registry.h"
#include "Vector2Int.h"
#include <gtest/gtest.h>

TEST(Registry, ViewReturnsOnlyEntitiesWithAllComponents) {
    domain::Registry registry;

    auto full = registry.entities.CreateEntity();
    registry.Storage<component::HealthComponent>().Add(full, {.current = 5, .max = 10});
    registry.Storage<component::MonsterTag>().Add(full, {});

    auto health_only = registry.entities.CreateEntity();
    registry.Storage<component::HealthComponent>().Add(health_only, {.current = 3, .max = 10});

    auto tag_only = registry.entities.CreateEntity();
    registry.Storage<component::MonsterTag>().Add(tag_only, {});

    auto view = registry.View<component::HealthComponent, component::MonsterTag>();

    ASSERT_EQ(view.size(), 1U);
    EXPECT_EQ(view[0], full);
}

TEST(Registry, ViewIsEmptyWhenNoEntityQualifies) {
    domain::Registry registry;

    auto id = registry.entities.CreateEntity();
    registry.Storage<component::HealthComponent>().Add(id, {.current = 5, .max = 10});

    auto view = registry.View<component::HealthComponent, component::MonsterTag>();

    EXPECT_TRUE(view.empty());
}

TEST(Registry, HasAllOnEmptyPackIsTrue) {
    domain::Registry registry;
    auto id = registry.entities.CreateEntity();

    EXPECT_TRUE(registry.HasAll<>(id));
}

TEST(Registry, GetRefReturnsMutableComponent) {
    domain::Registry registry;
    auto id = registry.entities.CreateEntity();
    registry.Storage<component::HealthComponent>().Add(id, {.current = 5, .max = 10});

    registry.GetRef<component::HealthComponent>(id).current -= 2;

    EXPECT_EQ(registry.Storage<component::HealthComponent>().Get(id)->current, 3);
}

TEST(RegistryDestroyEntity, RemovesEntityFromEveryStorage) {
    domain::Registry registry;
    auto id = registry.entities.CreateEntity();
    auto survivor = registry.entities.CreateEntity();

    registry.ForEachStorage([id, survivor](auto &storage) {
        storage.Add(id, {});
        storage.Add(survivor, {});
    });

    registry.DestroyEntity(id);

    registry.ForEachStorage([id, survivor](const auto &storage) {
        EXPECT_FALSE(storage.Has(id));
        EXPECT_TRUE(storage.Has(survivor));
    });
}

TEST(RegistryPlaceEntity, PlacingAnAlreadyPlacedEntityMovesItRatherThanBeingIgnored) {
    domain::Registry registry;
    auto id = registry.entities.CreateEntity();

    registry.PlaceEntity(id, {.coord = {.x = 1, .y = 2}, .map_level = 0});
    registry.PlaceEntity(id, {.coord = {.x = 7, .y = 4}, .map_level = 0});

    ASSERT_TRUE(registry.Storage<Location>().Has(id));
    EXPECT_EQ(registry.GetRef<Location>(id).coord, (Vector2Int{.x = 7, .y = 4}));
    int placed = 0;
    for (const auto &[placed_id, loc] : registry.Storage<Location>()) {
        (void)placed_id;
        (void)loc;
        placed++;
    }
    EXPECT_EQ(placed, 1);
}

TEST(RegistryPlaceEntity, ClearLocationLeavesTheEntityButUnplaced) {
    domain::Registry registry;
    auto id = registry.entities.CreateEntity();
    registry.Storage<component::HealthComponent>().Add(id, {.current = 3, .max = 3});
    registry.PlaceEntity(id, {.coord = {.x = 1, .y = 1}, .map_level = 0});

    registry.ClearLocation(id);

    EXPECT_FALSE(registry.Storage<Location>().Has(id));
    EXPECT_TRUE(registry.Storage<component::HealthComponent>().Has(id));
}
