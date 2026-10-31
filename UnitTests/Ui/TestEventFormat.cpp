#include "Components.h"
#include "EntityStorage.h"
#include "Event.h"
#include "EventFormat.h"
#include "Registry.h"
#include <array>
#include <gtest/gtest.h>
#include <string>

namespace {
entity::EntityId MakeNamed(domain::Registry &registry, const std::string &name) {
    auto id = registry.entities.CreateEntity();
    registry.Storage<component::NameComponent>().Add(id, {.name = name});
    return id;
}
} // namespace

TEST(EventFormat, MonsterSlainedUsesSnapshottedNamesForAGoneEntity) {
    const domain::Registry registry;
    const event::Event event{.type = event::Type::MonsterSlained,
                             .attacker = entity::EntityId{1},
                             .defender = entity::EntityId{2},
                             .attacker_name = "Hero",
                             .defender_name = "Rat"};

    const auto text = ui::FormatEvent(event, registry);

    EXPECT_NE(text.find("Hero"), std::string::npos);
    EXPECT_NE(text.find("Rat"), std::string::npos);
}

TEST(EventFormat, MonsterSlainedFallsBackToUnknownAttackerWhenUnset) {
    const domain::Registry registry;
    const event::Event event{.type = event::Type::MonsterSlained,
                             .defender = entity::EntityId{2},
                             .defender_name = "Rat"};

    EXPECT_NE(ui::FormatEvent(event, registry).find("Unknown"), std::string::npos);
}

TEST(EventFormat, HeroDiedUsesSnapshottedNameForAGoneEntity) {
    const domain::Registry registry;
    const event::Event event{
        .type = event::Type::HeroDied, .defender = entity::EntityId{1}, .defender_name = "Hero"};

    EXPECT_NE(ui::FormatEvent(event, registry).find("Hero"), std::string::npos);
}

TEST(EventFormat, UnhandledTypeFallsBackToDefaultText) {
    const domain::Registry registry;
    const event::Event event{.type = event::Type::Trade};
    EXPECT_EQ(ui::FormatEvent(event, registry), "No handler for event");
}

TEST(EventFormat, EveryLoggedTypeReturnsSomethingOtherThanTheFallback) {
    domain::Registry registry;
    auto attacker = MakeNamed(registry, "Attacker");
    auto defender = MakeNamed(registry, "Defender");

    const std::array logged_events = {
        event::Event{.type = event::Type::Missed, .attacker = attacker, .defender = defender},
        event::Event{.type = event::Type::NoWeapon},
        event::Event{.type = event::Type::NoAmmo},
        event::Event{.type = event::Type::NoMana},
        event::Event{.type = event::Type::GotDamage, .attacker = attacker, .defender = defender},
        event::Event{.type = event::Type::MonsterSlained,
                     .attacker_name = "Attacker",
                     .defender_name = "Defender"},
        event::Event{.type = event::Type::TrapDamage, .defender = defender},
        event::Event{.type = event::Type::TrapDisarmed, .defender = defender},
        event::Event{.type = event::Type::HeroDied, .defender_name = "Defender"},
        event::Event{.type = event::Type::Healed, .defender = defender},
        event::Event{.type = event::Type::NoItem},
        event::Event{.type = event::Type::ItemPickedUp, .defender_name = "Defender"},
        event::Event{.type = event::Type::ItemDropped, .defender = defender},
        event::Event{.type = event::Type::ItemUsed, .defender_name = "Potion"},
        event::Event{.type = event::Type::FullInventory},
        event::Event{.type = event::Type::Avoided, .attacker = attacker, .defender = defender},
        event::Event{.type = event::Type::Blocked, .attacker = attacker},
    };
    for (const auto &event : logged_events) {
        EXPECT_NE(ui::FormatEvent(event, registry), "No handler for event");
    }
}

TEST(FormatEventResult, MapsEachEventInOrder) {
    domain::Registry registry;
    auto attacker = MakeNamed(registry, "Hero");
    auto defender = MakeNamed(registry, "Rat");
    const event::Result result{
        .events = {
            event::Event{.type = event::Type::Missed, .attacker = attacker, .defender = defender},
            event::Event{.type = event::Type::GotDamage,
                         .attacker = attacker,
                         .defender = defender,
                         .value = 4},
            event::Event{.type = event::Type::MonsterSlained,
                         .attacker_name = "Hero",
                         .defender_name = "Rat"}}};

    const auto lines = ui::FormatEventResult(result, registry);

    ASSERT_EQ(lines.size(), 3U);
    EXPECT_EQ(lines[0], ui::FormatEvent(result.events[0], registry));
    EXPECT_EQ(lines[1], ui::FormatEvent(result.events[1], registry));
    EXPECT_EQ(lines[2], ui::FormatEvent(result.events[2], registry));
}

TEST(FormatEventResult, EmptyResultMapsToEmptyLines) {
    const domain::Registry registry;
    const event::Result result;
    EXPECT_TRUE(ui::FormatEventResult(result, registry).empty());
}

TEST(CountMonstersSlain, TwoMonsterSlainedEventsBumpTheCountByTwo) {
    const event::Result result{.events = {event::Event{.type = event::Type::MonsterSlained},
                                          event::Event{.type = event::Type::GotDamage, .value = 4},
                                          event::Event{.type = event::Type::MonsterSlained}}};

    EXPECT_EQ(ui::CountMonstersSlain(result), 2);
}

TEST(CountMonstersSlain, EmptyResultCountsZero) {
    const event::Result result;
    EXPECT_EQ(ui::CountMonstersSlain(result), 0);
}
