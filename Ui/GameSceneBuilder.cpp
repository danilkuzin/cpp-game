#include "GameSceneBuilder.h"
#include "Camera.h"
#include "Components.h"
#include "Coordinates.h"
#include "EntityStorage.h"
#include "Hero.h"
#include "Layout.h"
#include "Location.h"
#include "PanelLayout.h"
#include "PanelRects.h"
#include "RectInt.h"
#include "Registry.h"
#include "RegistryQueries.h"
#include "RenderScene.h"
#include "Vector2Int.h"
#include "WorldSceneBuilder.h"
#include <algorithm>
#include <optional>

namespace ui {

namespace {
constexpr int kFallbackMapLevel = 0;

RectInt HeroVisibleArea(const domain::Registry &registry) {
    const auto hero_id = systems::PlayerId(registry);
    if (!hero_id.has_value()) {
        return {};
    }
    const auto &loc = registry.GetRef<Location>(*hero_id);
    const auto view_zone = registry.GetRef<component::VisionComponent>(*hero_id).view_zone;
    return hero::GetVisibleArea(loc, view_zone);
}
} // namespace

void GameSceneBuilder::BuildEntities(RenderScene &scene, const domain::Registry &registry,
                                     const Camera &camera, RectInt visible_area, int map_level) {

    for (auto id : registry.View<component::RenderComponent, Location>()) {
        const auto &render = registry.GetRef<component::RenderComponent>(id);
        const auto &loc = registry.GetRef<Location>(id);
        if (loc.map_level == map_level && visible_area.ContainsPoint(loc.coord)) {
            scene.sprites.push_back(
                TileSprite(render.tile, loc.coord, camera.View(), render.z_index));
        }
    }
}

void GameSceneBuilder::BuildHover(RenderScene &scene, const domain::Registry &registry,
                                  const Camera &camera, Vector2Int tile) {
    const RectInt visible_area = HeroVisibleArea(registry);
    if (!visible_area.ContainsPoint(tile)) {
        return;
    }

    const auto hero_id =
        systems::PlayerId(registry).value(); // NOLINT(bugprone-unchecked-optional-access)
    const auto map_level = registry.GetRef<Location>(hero_id).map_level;

    const RectInt tile_rect = TileRect(tile, camera.View());
    scene.outlines.push_back({.rect = tile_rect, .color = {.r = 255, .g = 100, .b = 100}});

    std::optional<entity::EntityId> named_entity;
    int highest_z_index = 0;
    for (auto ent :
         systems::EntitiesAt(registry, Location{.coord = tile, .map_level = map_level})) {
        if (!registry.Storage<component::NameComponent>().Has(ent)) {
            continue;
        }
        const auto *render = registry.Storage<component::RenderComponent>().Get(ent);
        if (render == nullptr) {
            continue;
        }
        if (!named_entity.has_value() || render->z_index > highest_z_index) {
            named_entity = ent;
            highest_z_index = render->z_index;
        }
    }
    if (!named_entity.has_value()) {
        return;
    }

    const int tooltip_x = std::min(tile_rect.x + tile_rect.width,
                                   ui::layout::kScreenWidth - ui::layout::kTooltipWidth);
    const int tooltip_y =
        std::min(tile_rect.y, ui::layout::kScreenHeight - panel_layout::PanelHeight(1));
    scene.panels.push_back(
        {.rect = {.x = tooltip_x,
                  .y = tooltip_y,
                  .width = ui::layout::kTooltipWidth,
                  .height = panel_layout::PanelHeight(1)},
         .bg_color = ui::layout::kPanelBgColor,
         .text_color = ui::layout::kPanelTextColor,
         .rows = {registry.GetRef<component::NameComponent>(named_entity.value()).name}});
}

void GameSceneBuilder::BuildTargeting(RenderScene &scene, const domain::Registry &registry,
                                      const Camera &camera, Vector2Int direction) {
    const auto hero_id =
        systems::PlayerId(registry).value(); // NOLINT(bugprone-unchecked-optional-access)
    const auto ray = systems::SpellRayTiles(registry, hero_id, direction);

    for (const auto &tile : ray) {
        scene.outlines.push_back(
            {.rect = TileRect(tile, camera.View()), .color = {.r = 255, .g = 200, .b = 0}});
    }
}

void GameSceneBuilder::Build(RenderScene &scene, const domain::Registry &registry,
                             const Camera &camera) {
    const auto *map = systems::PlayerMap(registry);
    const auto &render_map = map != nullptr ? *map : registry.maps[kFallbackMapLevel];
    const auto hero_id = systems::PlayerId(registry);
    const int map_level =
        hero_id.has_value() ? registry.GetRef<Location>(*hero_id).map_level : kFallbackMapLevel;
    const RectInt visible_area = HeroVisibleArea(registry);
    WorldSceneBuilder::Build(scene, render_map, camera, registry.terrain_registry, visible_area);
    BuildEntities(scene, registry, camera, visible_area, map_level);
}
} // namespace ui
