#pragma once

#include "Bindings.h"
#include "EntityFactory.h"
#include "GameState.h"
#include "InputEvent.h"
#include "RenderScene.h"
#include "Session.h"
#include <cstdint>
#include <optional>

namespace game_states {
class CharacterGenState : public GameState {
public:
    const int kStartingMapLevel = 0;
    explicit CharacterGenState(Session &session);
    void OnEnter() override {};
    void OnExit() override {};
    GameState::Result Update() override;
    void BuildScene(ui::RenderScene &scene) const override;
    ui::InputContext InputContext() const override;
    std::optional<GameState::Result> OnAction(ui::Action action) override;
    void OnTextInput(ui::TextInputEvent) override;

private:
    game_states::Session &session_;
    enum class Stage : std::uint8_t {
        ChoosingClass,
        ChoosingRace,
        TypingName,
        Done
    } stage_ = Stage::ChoosingClass;

    entity::HeroGenerationParameters hero_generation_parameters_;
};
} // namespace game_states
