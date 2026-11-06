#include "EventFormat.h"
#include "Components.h"
#include "EntityStorage.h"
#include "Event.h"
#include "MessageLog.h"
#include "PanelRects.h"
#include "RectInt.h"
#include "Registry.h"
#include "RenderScene.h"
#include "Texts.h"
#include <algorithm>
#include <cassert>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace ui {
namespace {

std::string NameOf(const domain::Registry &registry, entity::EntityId id) {
    return registry.GetRef<component::NameComponent>(id).name;
}

std::string WithAttackerAndDefender(const std::string &label, const domain::Registry &registry,
                                    std::optional<entity::EntityId> attacker,
                                    std::optional<entity::EntityId> defender) {
    assert(attacker.has_value() && defender.has_value());
    return label + ", attacker: " + NameOf(registry, *attacker) +
           ", defender: " + NameOf(registry, *defender);
}

std::string WithDefender(const std::string &label, const domain::Registry &registry,
                         std::optional<entity::EntityId> defender) {
    assert(defender.has_value());
    return label + ", defender: " + NameOf(registry, *defender);
}

std::string FormatMonsterSlained(const event::Event &event) {
    assert(event.defender_name.has_value());
    return texts::StrMonKill() +
           ", attacker: " + event.attacker_name.value_or(texts::StrUnknownName()) +
           ", defender: " + *event.defender_name;
}

std::string FormatHeroDied(const event::Event &event) {
    assert(event.defender_name.has_value());
    return texts::StrHeroDied() + ", name: " + *event.defender_name;
}

std::string FormatItemUsed(const event::Event &event) {
    assert(event.defender_name.has_value());
    return texts::StrItemUsed() + ", defender: " + *event.defender_name;
}

std::string FormatItemPickedUp(const event::Event &event) {
    assert(event.defender_name.has_value());
    return texts::StrItemPickedUp() + ", defender: " + *event.defender_name;
}

} // namespace

std::string FormatEvent(const event::Event &event, const domain::Registry &registry) {
    switch (event.type) {
    case event::Type::Missed:
        return WithAttackerAndDefender(texts::StrBadAttack(), registry, event.attacker,
                                       event.defender);
    case event::Type::NoWeapon:
        return texts::StrNoneWeapons();
    case event::Type::NoAmmo:
        return texts::StrNoAmmo();
    case event::Type::NoMana:
        return texts::StrNoMana();
    case event::Type::GotDamage:
        return WithAttackerAndDefender(texts::StrAttack(), registry, event.attacker,
                                       event.defender) +
               ", damage: " + std::to_string(event.value);
    case event::Type::MonsterSlained:
        return FormatMonsterSlained(event);
    case event::Type::TrapDamage:
        return WithDefender(texts::StrTrap(), registry, event.defender) +
               ", damage: " + std::to_string(event.value);
    case event::Type::TrapDisarmed:
        return WithDefender(texts::StrTrapOk(), registry, event.defender);
    case event::Type::HeroDied:
        return FormatHeroDied(event);
    case event::Type::Healed:
        return WithDefender(texts::StrHealed(), registry, event.defender) +
               ", value: " + std::to_string(event.value);
    case event::Type::NoItem:
        return texts::StrEmptyItem();
    case event::Type::ItemPickedUp:
        return FormatItemPickedUp(event);
    case event::Type::ItemDropped:
        return WithDefender(texts::StrItemDropped(), registry, event.defender);
    case event::Type::ItemUsed:
        return FormatItemUsed(event);
    case event::Type::FullInventory:
        return texts::StrFullInventory();
    case event::Type::Avoided:
        return WithAttackerAndDefender(texts::StrMonStop(), registry, event.attacker,
                                       event.defender);
    case event::Type::Blocked:
        return texts::StrBlocked();
    default:
        return "No handler for event";
    }
}

std::vector<std::string> FormatEventResult(const event::Result &result,
                                           const domain::Registry &registry) {
    std::vector<std::string> res;
    res.reserve(result.events.size());
    for (const auto &event : result.events) {
        res.push_back(FormatEvent(event, registry));
    }
    return res;
}

int CountMonstersSlain(const event::Result &result) {
    return static_cast<int>(std::ranges::count_if(result.events, [](const event::Event &event) {
        return event.type == event::Type::MonsterSlained;
    }));
}

void PushLogPanel(ui::RenderScene &scene, const RectInt &rect, const MessageLog &log) {
    std::vector<std::string> log_lines{texts::LogLabel()};
    for (const auto &message : log) {
        log_lines.push_back(message);
    }
    scene.panels.push_back({.rect = rect,
                            .bg_color = ui::layout::kPanelBgColor,
                            .text_color = ui::layout::kPanelTextColor,
                            .rows = std::move(log_lines)});
}
} // namespace ui
