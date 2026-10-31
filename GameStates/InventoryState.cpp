#include "InventoryState.h"
#include "Bindings.h"
#include "EffectSystems.h"
#include "EntityStorage.h"
#include "Event.h"
#include "EventFormat.h"
#include "GameItem.h"
#include "GameState.h"
#include "InputEvent.h"
#include "Inventory.h"
#include "PanelRects.h"
#include "RenderScene.h"
#include "Session.h"
#include "SidebarPanel.h"
#include "Systems.h"
#include "Texts.h"
#include "Widget.h"
#include <array>
#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace game_states {
namespace {

constexpr std::array<game_item::EquipSlot, 6> kUiSlots = {
    game_item::EquipSlot::Chest,   game_item::EquipSlot::MainHand, game_item::EquipSlot::Head,
    game_item::EquipSlot::OffHand, game_item::EquipSlot::RingLeft, game_item::EquipSlot::RingRight};
} // namespace

InventoryState::InventoryState(Session &session) : session_(session) {}

void InventoryState::BuildScene(ui::RenderScene &scene) const {

    ui::PushSidebarBackdrop(scene);

    const auto hero_id = HeroId(session_);
    const auto item_ids = game_item::UnequippedItems(session_.registry, hero_id);

    std::vector<ui::ListRow> item_rows;
    item_rows.reserve(item_ids.size());
    for (auto item_id : item_ids) {
        auto line = "- " + session_.registry.GetRef<component::NameComponent>(item_id).name;
        if (session_.registry.Storage<game_item::ChargesComponent>().Has(item_id)) {
            line +=
                " (" +
                std::to_string(
                    session_.registry.Storage<game_item::ChargesComponent>().Get(item_id)->amount) +
                ")";
        }
        item_rows.push_back({.text = line, .entity = item_id});
    }
    ui::PushListPanel(scene, ui::layout::kSidebarUpperPanelRect, texts::StrHeroItems(), item_rows,
                      ui::WidgetKind::UpperRow);

    std::vector<std::optional<entity::EntityId>> slot_items;
    slot_items.assign(kUiSlots.size(), std::nullopt);
    for (std::size_t i = 0; i < kUiSlots.size(); ++i) {
        slot_items[i] = game_item::EquippedInSlot(session_.registry, hero_id, kUiSlots[i]);
    }

    std::vector<ui::ListRow> slot_rows;
    slot_rows.reserve(kUiSlots.size());
    for (std::size_t i = 0; i < kUiSlots.size(); ++i) {
        const auto &slot_item = slot_items[i];
        if (slot_item.has_value()) {
            auto line = texts::SlotName()[i] +
                        session_.registry.GetRef<component::NameComponent>(*slot_item).name;
            if (session_.registry.Storage<game_item::ChargesComponent>().Has(*slot_item)) {
                line += " (" +
                        std::to_string(session_.registry.Storage<game_item::ChargesComponent>()
                                           .Get(*slot_item)
                                           ->amount) +
                        ")";
            }
            slot_rows.push_back({.text = line, .entity = slot_item});
        } else {
            slot_rows.push_back(
                {.text = texts::SlotName()[i] + texts::StrEmptyItem(), .entity = std::nullopt});
        }
    }
    ui::PushListPanel(scene, ui::layout::kSidebarLowerPanelRect, texts::StrHeroSlots(), slot_rows,
                      ui::WidgetKind::LowerRow);

    ui::PushLogPanel(scene, ui::layout::kSidebarLogPanelRect, session_.runtime.log);

    scene.buttons.push_back({.rect = ui::layout::kSidebarCloseButtonRect,
                             .label = "Close",
                             .bg_color = ui::layout::kPanelBgColor,
                             .text_color = ui::layout::kPanelTextColor});
    scene.hit_regions.push_back({.rect = ui::layout::kSidebarCloseButtonRect,
                                 .id = {.kind = ui::WidgetKind::CloseButton,
                                        .entity = std::nullopt,
                                        .action = std::nullopt}});
}

void InventoryState::EquipItem(entity::EntityId item_id) {
    const auto hero_id = HeroId(session_);
    if (!game_item::EquipToFirstFittingSlot(session_.registry, {.item = item_id, .actor = hero_id},
                                            kUiSlots)) {
        Log(session_, texts::StrCantEquip());
    }
}

void InventoryState::DropItem(entity::EntityId item_id) {
    const auto hero_id = HeroId(session_);
    if (!game_item::Drop(session_.registry, {.item = item_id, .actor = hero_id})) {
        Log(session_, texts::StrCantDrop());
        return;
    }
    Log(session_,
        ui::FormatEvent(event::Event{.type = event::Type::ItemDropped, .defender = item_id},
                        session_.registry));
}

void InventoryState::UseItem(entity::EntityId item_id) {
    const auto hero_id = HeroId(session_);
    const auto *name = session_.registry.Storage<component::NameComponent>().Get(item_id);
    const auto item_name = name != nullptr ? name->name : std::string{};
    if (!game_item::UseItem(session_.registry, {.item = item_id, .actor = hero_id})) {
        Log(session_, texts::StrCantUse());
        return;
    }
    if (name != nullptr) {
        Log(session_,
            ui::FormatEvent(event::Event{.type = event::Type::ItemUsed, .defender_name = item_name},
                            session_.registry));
    }
    LogEvents(session_, systems::SpellSystem(session_.registry, session_.db));
    systems::DamageSystem(session_.registry);
    LogEvents(session_, systems::HealSystem(session_.registry));
    auto hero_pass_deaths = systems::DeathSystem(session_.registry, session_.db);
    LogEvents(session_, hero_pass_deaths);
    session_.stats.monsters_slain += ui::CountMonstersSlain(hero_pass_deaths);
}

std::optional<GameState::Result> InventoryState::OnWidgetClick(ui::WidgetId widget,
                                                               ui::MouseButton button) {
    switch (widget.kind) {
    case ui::WidgetKind::UpperRow:
        if (!widget.entity.has_value()) {
            break;
        }
        if (button == ui::MouseButton::Left) {
            EquipItem(widget.entity.value());
        } else if (button == ui::MouseButton::Right) {
            DropItem(widget.entity.value());
        } else if (button == ui::MouseButton::Middle) {
            UseItem(widget.entity.value());
        }
        break;
    case ui::WidgetKind::LowerRow:
        if (!widget.entity.has_value()) {
            break;
        }
        if (button == ui::MouseButton::Left) {
            game_item::Unequip(session_.registry, widget.entity.value());
        } else if (button == ui::MouseButton::Middle) {
            UseItem(widget.entity.value());
        }
        break;
    case ui::WidgetKind::CloseButton:
        if (button == ui::MouseButton::Left) {
            return GameState::Result{.type = GameState::Result::Type::Pop, .next_state = nullptr};
        }
        break;
    default:
        break;
    }
    return std::nullopt;
}

ui::InputContext InventoryState::InputContext() const { return ui::InputContext::Menu; }

std::optional<GameState::Result> InventoryState::OnAction(ui::Action action) {
    switch (action) {
    case ui::Action::OpenInventory:
        return GameState::Result{.type = GameState::Result::Type::Pop, .next_state = nullptr};

    default:
        break;
    }
    return std::nullopt;
}
} // namespace game_states
