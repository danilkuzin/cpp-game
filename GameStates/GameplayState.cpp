#include "GameplayState.h"
#include "Bindings.h"
#include "ComponentStorage.h"
#include "Components.h"
#include "Coordinates.h"
#include "EffectSystems.h"
#include "Event.h"
#include "EventFormat.h"
#include "GameDatabase.h"
#include "GameOverState.h"
#include "GameSceneBuilder.h"
#include "GameState.h"
#include "Hero.h"
#include "HeroInfoState.h"
#include "InputEvent.h"
#include "Inventory.h"
#include "InventoryState.h"
#include "Location.h"
#include "PanelRects.h"
#include "RegistryQueries.h"
#include "RenderScene.h"
#include "Session.h"
#include "Systems.h"
#include "Texts.h"
#include "TradeState.h"
#include "Vector2Int.h"
#include "Widget.h"
#include <algorithm>
#include <array>
#include <cstddef>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace game_states {
GameplayState::GameplayState(Session &session) : session_(session) {}

namespace {
bool HeroDied(const event::Result &result) {
    return std::ranges::any_of(result.events, [](const event::Event &event) {
        return event.type == event::Type::HeroDied;
    });
}

bool ActionFailed(const event::Result &result) {
    return std::ranges::any_of(result.events, [](const event::Event &event) {
        switch (event.type) {
        case event::Type::NoWeapon:
        case event::Type::NoAmmo:
        case event::Type::NoMana:
        case event::Type::NoItem:
        case event::Type::FullInventory:
        case event::Type::Blocked:
            return true;
        default:
            return false;
        }
    });
}

template <typename T>
void ThrowIfLeaked(const component::ComponentStorage<T> &storage, std::string_view name) {
    if (!storage.Empty()) {
        throw(
            std::runtime_error(std::string("Non-empty intent at the end of turn: ").append(name)));
    }
}

constexpr std::array<std::string_view, 2> kSpellBar{"fireball", "healing_light"};
static_assert(kSpellBar.size() == ui::layout::kSpellBarSlots);

} // namespace

ui::InputContext GameplayState::InputContext() const { return ui::InputContext::Gameplay; }

void GameplayState::ConfirmTargeting() {
    auto &mode = targeting_.value(); // NOLINT(bugprone-unchecked-optional-access)

    session_.registry.Storage<component::IntentToCastSpell>().Add(
        HeroId(session_), {.spell_id = std::string(kSpellBar[mode.spell_slot]),
                           .costs_mana = true,
                           .direction = mode.direction});
    pending_.intent_stamped = true;
    targeting_.reset();
}

void GameplayState::CastSpell(std::size_t slot) {
    const auto spell_id = std::string(kSpellBar[slot]);
    const auto &spell = session_.db.GetSpell(spell_id);

    if (spell.damage.has_value()) {
        const auto &hero_location = session_.registry.GetRef<Location>(HeroId(session_));
        targeting_ = {.spell_slot = slot, .origin = hero_location.coord};
        return;
    }

    session_.registry.Storage<component::IntentToCastSpell>().Add(
        HeroId(session_), {.spell_id = spell_id, .costs_mana = true, .direction = std::nullopt});
    pending_.intent_stamped = true;
}

std::optional<GameState::Result> GameplayState::OnTargetingAction(ui::Action action) {
    auto &mode = targeting_.value(); // NOLINT(bugprone-unchecked-optional-access)

    switch (action) {
    case ui::Action::MoveNorth:
        mode.direction = Vector2Int::Up();
        break;
    case ui::Action::MoveSouth:
        mode.direction = Vector2Int::Down();
        break;
    case ui::Action::MoveWest:
        mode.direction = Vector2Int::Left();
        break;
    case ui::Action::MoveEast:
        mode.direction = Vector2Int::Right();
        break;
    case ui::Action::CastSpell1:
        mode.spell_slot = 0;
        break;
    case ui::Action::CastSpell2:
        mode.spell_slot = 1;
        break;
    case ui::Action::Confirm: {
        ConfirmTargeting();
        break;
    }
    case ui::Action::Cancel:
        targeting_.reset();
        break;
    default:
        break;
    }
    return std::nullopt;
}

std::optional<GameState::Result> GameplayState::OnWidgetClick(ui::WidgetId widget,
                                                              ui::MouseButton button) {
    switch (widget.kind) {
    case ui::WidgetKind::SpellButton:
        if (button == ui::MouseButton::Left && widget.action.has_value()) {
            return OnAction(widget.action.value());
        }
        break;
    default:
        break;
    }
    return std::nullopt;
}

std::optional<GameState::Result> GameplayState::OnAction(ui::Action action) {
    if (targeting_.has_value()) {
        return OnTargetingAction(action);
    }

    switch (action) {
    case ui::Action::OpenHeroInfo:
        return GameState::Result{.type = GameState::Result::Type::Push,
                                 .next_state = std::make_unique<HeroInfoState>(session_)};

    case ui::Action::OpenInventory:
        return GameState::Result{.type = GameState::Result::Type::Push,
                                 .next_state = std::make_unique<InventoryState>(session_)};

    case ui::Action::MoveNorth:
        session_.registry.Storage<component::IntentToMove>().Add(HeroId(session_),
                                                                 {Vector2Int::Up()});
        pending_.intent_stamped = true;
        break;
    case ui::Action::MoveSouth:
        session_.registry.Storage<component::IntentToMove>().Add(HeroId(session_),
                                                                 {Vector2Int::Down()});
        pending_.intent_stamped = true;
        break;
    case ui::Action::MoveWest:
        session_.registry.Storage<component::IntentToMove>().Add(HeroId(session_),
                                                                 {Vector2Int::Left()});
        pending_.intent_stamped = true;
        break;
    case ui::Action::MoveEast:
        session_.registry.Storage<component::IntentToMove>().Add(HeroId(session_),
                                                                 {Vector2Int::Right()});
        pending_.intent_stamped = true;
        break;
    case ui::Action::ShootNorth:
        session_.registry.Storage<component::IntentToShoot>().Add(HeroId(session_),
                                                                  {Vector2Int::Up()});
        pending_.intent_stamped = true;
        break;
    case ui::Action::ShootWest:
        session_.registry.Storage<component::IntentToShoot>().Add(HeroId(session_),
                                                                  {Vector2Int::Left()});
        pending_.intent_stamped = true;
        break;
    case ui::Action::ShootEast:
        session_.registry.Storage<component::IntentToShoot>().Add(HeroId(session_),
                                                                  {Vector2Int::Right()});
        pending_.intent_stamped = true;
        break;
    case ui::Action::ShootSouth:
        session_.registry.Storage<component::IntentToShoot>().Add(HeroId(session_),
                                                                  {Vector2Int::Down()});
        pending_.intent_stamped = true;
        break;
    case ui::Action::CastSpell1:
        CastSpell(0);
        break;
    case ui::Action::CastSpell2:
        CastSpell(1);
        break;
    case ui::Action::PickUp: {
        auto pickup_result = game_item::PickUpFromTile(session_.registry, HeroId(session_));
        LogEvents(session_, pickup_result);
        pending_.took_turn = !ActionFailed(pickup_result);
        pending_.pickup_attempted = true;
        break;
    }
    default:
        break;
    }
    return std::nullopt;
}

void GameplayState::OnMouseClick(ui::MouseClickEvent event) {
    if (!targeting_.has_value() || event.button != ui::MouseButton::Left) {
        return;
    }
    const auto tile = ui::ScreenToTile(event.x, event.y, session_.runtime.camera.View());
    if (!tile.has_value()) {
        return;
    }
    const auto aim = ui::CardinalToward(targeting_->origin, tile.value());
    if (!aim.has_value()) {
        return;
    }
    targeting_->direction = aim.value();
    ConfirmTargeting();
}

void GameplayState::OnMouseMove(ui::MouseMoveEvent event, std::optional<ui::WidgetId> widget) {
    if (widget.has_value()) {
        hover_tile_.reset();
        return;
    }
    hover_tile_ = ui::ScreenToTile(event.x, event.y, session_.runtime.camera.View());
    if (targeting_.has_value() && hover_tile_.has_value()) {
        if (auto aim = ui::CardinalToward(targeting_->origin, hover_tile_.value());
            aim.has_value()) {
            targeting_->direction = aim.value();
        }
    }
}

void GameplayState::RefreshStats() {
    auto hero_id = HeroId(session_);
    session_.stats.hero_name =
        session_.registry.Storage<component::NameComponent>().Get(hero_id)->name;
    session_.stats.level = session_.registry.Storage<hero::HeroExperience>().Get(hero_id)->level;
    session_.stats.money = game_item::Money(session_.registry, hero_id);
    session_.stats.equipment.clear();
    for (auto i : game_item::EquippedItems(session_.registry, hero_id)) {
        session_.stats.equipment.push_back(
            session_.registry.Storage<component::NameComponent>().Get(i)->name);
    }
}

GameState::Result GameplayState::Update() {
    bool hero_died = false;

    if (pending_.intent_stamped || pending_.took_turn) {
        RefreshStats();

        if (pending_.intent_stamped) {
            auto collision_result = systems::CollisionSystem(session_.registry);
            for (const auto &collision_event : collision_result.events) {
                if (collision_event.type == event::Type::Trade) {
                    ThrowIfLeaked(session_.registry.Storage<component::IntentToMove>(),
                                  "IntentToMove");
                    ThrowIfLeaked(session_.registry.Storage<component::IntentToMelee>(),
                                  "IntentToMelee");
                    const auto &defender = collision_event.defender;
                    if (defender.has_value()) {
                        return GameState::Result{
                            .type = GameState::Result::Type::Push,
                            .next_state = std::make_unique<TradeState>(session_, *defender)};
                    }
                }
            }
            systems::OpportunityAttackSystem(session_.registry);
            auto melee_result = systems::MeleeSystem(session_.registry);
            LogEvents(session_, melee_result);
            auto ranged_result = systems::RangedSystem(session_.registry);
            LogEvents(session_, ranged_result);
            auto spell_result = systems::SpellSystem(session_.registry, session_.db);
            LogEvents(session_, spell_result);
            auto movement_result = systems::MovementSystem(session_.registry);
            LogEvents(session_, movement_result);

            pending_.took_turn = !ActionFailed(collision_result) && !ActionFailed(melee_result) &&
                                 !ActionFailed(ranged_result) && !ActionFailed(spell_result) &&
                                 !ActionFailed(movement_result);
        }
        if (const auto *loc = session_.registry.Storage<Location>().Get(HeroId(session_))) {
            if (session_.runtime.camera.IsCloseToEdge(loc->coord)) {
                session_.runtime.camera.CenterAt(loc->coord);
            }
        }
        systems::DamageSystem(session_.registry);
        LogEvents(session_, systems::HealSystem(session_.registry));
        auto hero_pass_deaths = systems::DeathSystem(session_.registry, session_.db);
        LogEvents(session_, hero_pass_deaths);
        session_.stats.monsters_slain += ui::CountMonstersSlain(hero_pass_deaths);
        hero_died = HeroDied(hero_pass_deaths);
    }
    if (pending_.took_turn) {
        systems::AISystem(session_.registry);
        systems::CollisionSystem(session_.registry);
        LogEvents(session_, systems::MeleeSystem(session_.registry));
        LogEvents(session_, systems::MovementSystem(session_.registry));
        systems::DamageSystem(session_.registry);
        auto monster_pass_deaths = systems::DeathSystem(session_.registry, session_.db);
        LogEvents(session_, monster_pass_deaths);
        session_.stats.monsters_slain += ui::CountMonstersSlain(monster_pass_deaths);
        hero_died = hero_died || HeroDied(monster_pass_deaths);
    }

    ThrowIfLeaked(session_.registry.Storage<component::IntentToMove>(), "IntentToMove");
    ThrowIfLeaked(session_.registry.Storage<component::IntentToMelee>(), "IntentToMelee");
    ThrowIfLeaked(session_.registry.Storage<component::IntentToShoot>(), "IntentToShoot");
    ThrowIfLeaked(session_.registry.Storage<component::IntentToTakeDamage>(), "IntentToTakeDamage");
    ThrowIfLeaked(session_.registry.Storage<component::IntentToHeal>(), "IntentToHeal");
    ThrowIfLeaked(session_.registry.Storage<component::IntentToCastSpell>(), "IntentToCastSpell");
    pending_ = {};

    if (hero_died) {
        return GameState::Result{.type = GameState::Result::Type::Push,
                                 .next_state = std::make_unique<GameOverState>(session_)};
    }
    return GameState::Result{.type = GameState::Result::Type::Continue, .next_state = nullptr};
}

void GameplayState::BuildScene(ui::RenderScene &scene) const {
    ui::GameSceneBuilder::Build(scene, session_.registry, session_.runtime.camera);

    scene.hit_regions.push_back({.rect = ui::layout::kGameplayInfoPanelRect, .id = {}});
    scene.hit_regions.push_back({.rect = ui::layout::kGameplayLogPanelRect, .id = {}});

    const auto hero_id = systems::PlayerId(session_.registry);
    if (!hero_id.has_value()) {
        scene.panels.push_back({.rect = ui::layout::kGameplayInfoPanelRect,
                                .bg_color = ui::layout::kPanelBgColor,
                                .text_color = ui::layout::kPanelTextColor,
                                .rows = {texts::StrHeroDied()}});
    } else {
        const auto &hero_location = session_.registry.GetRef<Location>(*hero_id);
        const auto &hero_hp = session_.registry.GetRef<component::HealthComponent>(*hero_id);
        const auto &hero_mana = session_.registry.GetRef<component::ManaComponent>(*hero_id);

        scene.panels.push_back(
            {.rect = ui::layout::kGameplayInfoPanelRect,
             .bg_color = ui::layout::kPanelBgColor,
             .text_color = ui::layout::kPanelTextColor,
             .rows = {texts::StrHeroHp() + std::to_string(hero_hp.current) + "/" +
                          std::to_string(hero_hp.max),
                      texts::StrHeroMana() + std::to_string(hero_mana.current) + "/" +
                          std::to_string(hero_mana.max),
                      texts::StrHeroXy() + std::to_string(hero_location.coord.x) + "/" +
                          std::to_string(hero_location.coord.y),
                      texts::StrHeroMoney() +
                          std::to_string(game_item::Money(session_.registry, *hero_id))}});
    }

    ui::PushLogPanel(scene, ui::layout::kGameplayLogPanelRect, session_.runtime.log);

    if (hover_tile_.has_value()) {
        ui::GameSceneBuilder::BuildHover(scene, session_.registry, session_.runtime.camera,
                                         hover_tile_.value());
    }

    if (targeting_.has_value() && hero_id.has_value()) {
        ui::GameSceneBuilder::BuildTargeting(scene, session_.registry, session_.runtime.camera,
                                             targeting_->direction);
    }

    const bool spell1_aimed = targeting_.has_value() && targeting_->spell_slot == 0;
    const auto spell_btn0 = ui::layout::SpellButtonRect(0);
    scene.buttons.push_back(
        {.rect = spell_btn0,
         .label = "1: " + session_.db.GetSpell(std::string(kSpellBar[0])).name,
         .bg_color = spell1_aimed ? ui::layout::kSpellButtonAimedColor : ui::layout::kPanelBgColor,
         .text_color = ui::layout::kPanelTextColor});
    scene.hit_regions.push_back({.rect = spell_btn0,
                                 .id = {.kind = ui::WidgetKind::SpellButton,
                                        .entity = std::nullopt,
                                        .action = ui::Action::CastSpell1}});

    const bool spell2_aimed = targeting_.has_value() && targeting_->spell_slot == 1;
    const auto spell_btn1 = ui::layout::SpellButtonRect(1);
    scene.buttons.push_back(
        {.rect = spell_btn1,
         .label = "2: " + session_.db.GetSpell(std::string(kSpellBar[1])).name,
         .bg_color = spell2_aimed ? ui::layout::kSpellButtonAimedColor : ui::layout::kPanelBgColor,
         .text_color = ui::layout::kPanelTextColor});
    scene.hit_regions.push_back({.rect = spell_btn1,
                                 .id = {.kind = ui::WidgetKind::SpellButton,
                                        .entity = std::nullopt,
                                        .action = ui::Action::CastSpell2}});
}
} // namespace game_states
