#include "Camera.h"
#include "Components.h"
#include "Coordinates.h"
#include "GameSceneBuilder.h"
#include "Layout.h"
#include "Location.h"
#include "RectInt.h"
#include "Registry.h"
#include "RegistryQueries.h"
#include "RenderScene.h"
#include "Support/MakeRegistry.h"
#include "Vector2Int.h"
#include "WorldSceneBuilder.h"
#include <cstddef>
#include <gtest/gtest.h>

namespace {
Vector2Int SpriteCoord(const ui::RenderSprite &sprite, const ui::Camera &camera) {
    return {.x = (sprite.x / ui::layout::kTileWidth) - ui::layout::kWindowLeft + camera.View().x,
            .y = (sprite.y / ui::layout::kTileHeight) - ui::layout::kWindowTop + camera.View().y};
}

void ExpectOutlineMatchesTile(const ui::RenderOutline &outline, Vector2Int tile,
                              const ui::Camera &camera) {
    const auto expected_pixel = ui::TileToScreen(tile, camera.View());
    EXPECT_EQ(outline.rect.x, expected_pixel.x);
    EXPECT_EQ(outline.rect.y, expected_pixel.y);
    EXPECT_EQ(outline.rect.width, ui::layout::kTileWidth);
    EXPECT_EQ(outline.rect.height, ui::layout::kTileHeight);
}
} // namespace

TEST(WorldSceneBuilder, UnexploredTilesAreSkipped) {
    auto registry = MakeTestRegistry();
    const ui::Camera camera(RectInt{.x = 4, .y = 4, .width = 4, .height = 4}, 1);
    ui::RenderScene scene;

    ui::WorldSceneBuilder::Build(scene, registry.maps[0], camera, registry.terrain_registry,
                                 RectInt{});

    EXPECT_TRUE(scene.sprites.empty());
}

TEST(WorldSceneBuilder, ExploredButUnseenIsDim) {
    auto registry = MakeTestRegistry();
    const ui::Camera camera(RectInt{.x = 4, .y = 4, .width = 4, .height = 4}, 1);
    registry.maps[0].SetAreaVisible(camera.View());
    const RectInt visible_area{.x = 5, .y = 5, .width = 2, .height = 2};
    ui::RenderScene scene;

    ui::WorldSceneBuilder::Build(scene, registry.maps[0], camera, registry.terrain_registry,
                                 visible_area);

    ASSERT_EQ(scene.sprites.size(), 16U);
    for (const auto &sprite : scene.sprites) {
        const bool inside = visible_area.ContainsPoint(SpriteCoord(sprite, camera));
        EXPECT_EQ(sprite.dim, !inside);
    }
}

namespace {
domain::Registry MakeHeroWorld(Vector2Int hero_coord, int view_zone) {
    auto registry = MakeTestRegistry();
    const auto hero_id = registry.entities.CreateEntity();
    registry.Storage<Location>().Add(hero_id, {.coord = hero_coord, .map_level = 0});
    registry.Storage<component::VisionComponent>().Add(hero_id, {.view_zone = view_zone});
    registry.Storage<component::PlayerTag>().Add(hero_id, {});
    return registry;
}

ui::Camera MakeTestCamera() {
    return ui::Camera(RectInt{.x = 4, .y = 4, .width = 4, .height = 4}, 1);
}
} // namespace

TEST(GameSceneBuilder, EntitiesOutsideVisionRectAreNotDrawn) {
    auto registry = MakeHeroWorld({.x = 6, .y = 6}, 1);
    const auto camera = MakeTestCamera();

    const auto visible_monster = registry.entities.CreateEntity();
    registry.Storage<Location>().Add(visible_monster, {.coord = {.x = 6, .y = 6}, .map_level = 0});
    registry.Storage<component::RenderComponent>().Add(
        visible_monster, {.tile = "visible_monster.bmp", .z_index = ui::layout::kMonsterZIndex});

    const auto hidden_monster = registry.entities.CreateEntity();
    registry.Storage<Location>().Add(hidden_monster, {.coord = {.x = 4, .y = 4}, .map_level = 0});
    registry.Storage<component::RenderComponent>().Add(
        hidden_monster, {.tile = "hidden_monster.bmp", .z_index = ui::layout::kMonsterZIndex});

    ui::RenderScene scene;
    ui::GameSceneBuilder::Build(scene, registry, camera);

    ASSERT_EQ(scene.sprites.size(), 1U);
    EXPECT_EQ(scene.sprites[0].sprite, "visible_monster.bmp");
}

TEST(GameSceneBuilder, NoHeroOnActiveMapMeansNothingIsUndimmed) {
    auto registry = MakeTestRegistry();
    const auto camera = MakeTestCamera();
    registry.maps[0].SetAreaVisible(camera.View());

    ui::RenderScene scene;
    ui::GameSceneBuilder::Build(scene, registry, camera);

    ASSERT_FALSE(scene.sprites.empty());
    for (const auto &sprite : scene.sprites) {
        EXPECT_TRUE(sprite.dim);
    }
}

TEST(GameSceneBuilder, EntityOnAnotherMapLevelIsNotDrawn) {
    auto registry = MakeHeroWorld({.x = 6, .y = 6}, 1);
    const auto camera = MakeTestCamera();

    const auto other_level_monster = registry.entities.CreateEntity();
    registry.Storage<Location>().Add(other_level_monster,
                                     {.coord = {.x = 6, .y = 6}, .map_level = 1});
    registry.Storage<component::RenderComponent>().Add(
        other_level_monster,
        {.tile = "other_level_monster.bmp", .z_index = ui::layout::kMonsterZIndex});

    ui::RenderScene scene;
    ui::GameSceneBuilder::Build(scene, registry, camera);

    EXPECT_TRUE(scene.sprites.empty());
}

TEST(GameSceneBuilder, BuildHoverOutlinesTheHoveredTile) {
    auto registry = MakeHeroWorld({.x = 6, .y = 6}, 1);
    const auto camera = MakeTestCamera();

    ui::RenderScene scene;
    ui::GameSceneBuilder::BuildHover(scene, registry, camera, {.x = 6, .y = 6});

    ASSERT_EQ(scene.outlines.size(), 1U);
    const auto expected_pixel = ui::TileToScreen({.x = 6, .y = 6}, camera.View());
    EXPECT_EQ(scene.outlines[0].rect.x, expected_pixel.x);
    EXPECT_EQ(scene.outlines[0].rect.y, expected_pixel.y);
    EXPECT_EQ(scene.outlines[0].rect.width, ui::layout::kTileWidth);
    EXPECT_EQ(scene.outlines[0].rect.height, ui::layout::kTileHeight);
}

TEST(GameSceneBuilder, BuildHoverNamesTheEntityOnTheHoveredTile) {
    auto registry = MakeHeroWorld({.x = 6, .y = 6}, 1);
    const auto camera = MakeTestCamera();
    const auto monster_id = registry.entities.CreateEntity();
    registry.Storage<Location>().Add(monster_id, {.coord = {.x = 6, .y = 6}, .map_level = 0});
    registry.Storage<component::RenderComponent>().Add(
        monster_id, {.tile = "goblin.bmp", .z_index = ui::layout::kMonsterZIndex});
    registry.Storage<component::NameComponent>().Add(monster_id, {.name = "Goblin"});

    ui::RenderScene scene;
    ui::GameSceneBuilder::BuildHover(scene, registry, camera, {.x = 6, .y = 6});

    ASSERT_EQ(scene.panels.size(), 1U);
    ASSERT_EQ(scene.panels[0].rows.size(), 1U);
    EXPECT_EQ(scene.panels[0].rows[0], "Goblin");
}

TEST(GameSceneBuilder, BuildHoverNamesNothingOnAnEmptyTile) {
    auto registry = MakeHeroWorld({.x = 6, .y = 6}, 1);
    const auto camera = MakeTestCamera();

    ui::RenderScene scene;
    ui::GameSceneBuilder::BuildHover(scene, registry, camera, {.x = 6, .y = 6});

    EXPECT_EQ(scene.outlines.size(), 1U);
    EXPECT_TRUE(scene.panels.empty());
}

TEST(GameSceneBuilder, BuildHoverDoesNothingWithNoHero) {
    auto registry = MakeTestRegistry();
    const auto camera = MakeTestCamera();

    ui::RenderScene scene;
    EXPECT_NO_THROW(ui::GameSceneBuilder::BuildHover(scene, registry, camera, {.x = 6, .y = 6}));

    EXPECT_TRUE(scene.outlines.empty());
    EXPECT_TRUE(scene.panels.empty());
}

TEST(GameSceneBuilder, BuildHoverNamesNothingOutsideTheHerosVision) {
    auto registry = MakeHeroWorld({.x = 6, .y = 6}, 1);
    const auto camera = MakeTestCamera();
    const auto monster_id = registry.entities.CreateEntity();
    registry.Storage<Location>().Add(monster_id, {.coord = {.x = 4, .y = 4}, .map_level = 0});
    registry.Storage<component::RenderComponent>().Add(
        monster_id, {.tile = "goblin.bmp", .z_index = ui::layout::kMonsterZIndex});
    registry.Storage<component::NameComponent>().Add(monster_id, {.name = "Goblin"});

    ui::RenderScene scene;
    ui::GameSceneBuilder::BuildHover(scene, registry, camera, {.x = 4, .y = 4});

    EXPECT_TRUE(scene.panels.empty());
}

TEST(GameSceneBuilder, BuildHoverPushesNoHitRegion) {
    auto registry = MakeHeroWorld({.x = 6, .y = 6}, 1);
    const auto camera = MakeTestCamera();
    const auto monster_id = registry.entities.CreateEntity();
    registry.Storage<Location>().Add(monster_id, {.coord = {.x = 6, .y = 6}, .map_level = 0});
    registry.Storage<component::RenderComponent>().Add(
        monster_id, {.tile = "goblin.bmp", .z_index = ui::layout::kMonsterZIndex});
    registry.Storage<component::NameComponent>().Add(monster_id, {.name = "Goblin"});

    ui::RenderScene scene;
    ui::GameSceneBuilder::BuildHover(scene, registry, camera, {.x = 6, .y = 6});

    EXPECT_TRUE(scene.hit_regions.empty());
}

TEST(GameSceneBuilder, TooltipIsClampedToTheScreen) {
    auto registry = MakeHeroWorld({.x = 26, .y = 26}, 1);
    const ui::Camera camera{RectInt{.x = 0, .y = 0, .width = 30, .height = 30}, 1};
    const auto monster_id = registry.entities.CreateEntity();
    registry.Storage<Location>().Add(monster_id, {.coord = {.x = 26, .y = 26}, .map_level = 0});
    registry.Storage<component::RenderComponent>().Add(
        monster_id, {.tile = "goblin.bmp", .z_index = ui::layout::kMonsterZIndex});
    registry.Storage<component::NameComponent>().Add(monster_id, {.name = "Goblin"});

    ui::RenderScene scene;
    ui::GameSceneBuilder::BuildHover(scene, registry, camera, {.x = 26, .y = 26});

    ASSERT_EQ(scene.panels.size(), 1U);
    const auto &rect = scene.panels[0].rect;
    EXPECT_LE(rect.x + rect.width, ui::layout::kScreenWidth);
    EXPECT_LE(rect.y + rect.height, ui::layout::kScreenHeight);
}

TEST(GameSceneBuilder, BuildTargetingOutlinesTheAimedRay) {
    auto registry = MakeHeroWorld({.x = 2, .y = 2}, 1);
    const auto camera = MakeTestCamera();

    ui::RenderScene scene;
    ui::GameSceneBuilder::BuildTargeting(scene, registry, camera, Vector2Int::Right());

    ASSERT_EQ(scene.outlines.size(), static_cast<std::size_t>(systems::kSpellRange));
    for (std::size_t i = 0; i < scene.outlines.size(); ++i) {
        const Vector2Int tile{.x = 2 + static_cast<int>(i) + 1, .y = 2};
        ExpectOutlineMatchesTile(scene.outlines[i], tile, camera);
    }
}

TEST(GameSceneBuilder, BuildTargetingPushesNoHitRegion) {
    auto registry = MakeHeroWorld({.x = 2, .y = 2}, 1);
    const auto camera = MakeTestCamera();

    ui::RenderScene scene;
    ui::GameSceneBuilder::BuildTargeting(scene, registry, camera, Vector2Int::Right());

    EXPECT_TRUE(scene.hit_regions.empty());
}
