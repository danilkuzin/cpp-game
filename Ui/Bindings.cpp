#include "Bindings.h"
#include "InputEvent.h"
#include <array>
#include <optional>

namespace ui {
namespace {
struct Resolver {
    InputContext input_context = InputContext::Gameplay;
    Key key = Key::W;
    Action action = Action::MoveNorth;
};

constexpr std::array<Resolver, 25> kResolvers{
    Resolver{.input_context = InputContext::Gameplay, .key = Key::Up, .action = Action::MoveNorth},
    Resolver{
        .input_context = InputContext::Gameplay, .key = Key::Down, .action = Action::MoveSouth},
    Resolver{.input_context = InputContext::Gameplay, .key = Key::Left, .action = Action::MoveWest},
    Resolver{
        .input_context = InputContext::Gameplay, .key = Key::Right, .action = Action::MoveEast},
    Resolver{.input_context = InputContext::Gameplay, .key = Key::W, .action = Action::ShootNorth},
    Resolver{.input_context = InputContext::Gameplay, .key = Key::Z, .action = Action::ShootSouth},
    Resolver{.input_context = InputContext::Gameplay, .key = Key::A, .action = Action::ShootWest},
    Resolver{.input_context = InputContext::Gameplay, .key = Key::D, .action = Action::ShootEast},
    Resolver{
        .input_context = InputContext::Gameplay, .key = Key::Num1, .action = Action::CastSpell1},
    Resolver{
        .input_context = InputContext::Gameplay, .key = Key::Num2, .action = Action::CastSpell2},
    Resolver{.input_context = InputContext::Gameplay, .key = Key::P, .action = Action::PickUp},
    Resolver{
        .input_context = InputContext::Gameplay, .key = Key::I, .action = Action::OpenInventory},
    Resolver{
        .input_context = InputContext::Gameplay, .key = Key::H, .action = Action::OpenHeroInfo},
    Resolver{.input_context = InputContext::Gameplay, .key = Key::Escape, .action = Action::Cancel},
    Resolver{.input_context = InputContext::Menu, .key = Key::Escape, .action = Action::Cancel},
    Resolver{.input_context = InputContext::Menu, .key = Key::Return, .action = Action::Confirm},
    Resolver{.input_context = InputContext::Menu, .key = Key::Space, .action = Action::Confirm},
    Resolver{.input_context = InputContext::Menu, .key = Key::Up, .action = Action::PrevOption},
    Resolver{.input_context = InputContext::Menu, .key = Key::Down, .action = Action::NextOption},
    Resolver{.input_context = InputContext::Menu, .key = Key::I, .action = Action::OpenInventory},
    Resolver{.input_context = InputContext::Menu, .key = Key::H, .action = Action::OpenHeroInfo},
    Resolver{
        .input_context = InputContext::TextEntry, .key = Key::Return, .action = Action::Confirm},
    Resolver{.input_context = InputContext::TextEntry,
             .key = Key::Backspace,
             .action = Action::DeleteChar},
    Resolver{
        .input_context = InputContext::TextEntry, .key = Key::Escape, .action = Action::Cancel},
    Resolver{
        .input_context = InputContext::Gameplay, .key = Key::Return, .action = Action::Confirm},
};
} // namespace

std::optional<Action> Resolve(InputContext context, Key key) {
    for (auto r : kResolvers) {
        if (r.input_context == context && r.key == key) {
            return r.action;
        }
    }
    return std::nullopt;
}
} // namespace ui
