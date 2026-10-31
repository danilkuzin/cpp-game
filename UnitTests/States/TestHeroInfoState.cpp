#include "Bindings.h"
#include "Components.h"
#include "GameState.h"
#include "Hero.h"
#include "HeroInfoState.h"
#include "RenderScene.h"
#include "Support/MakeSession.h"
#include "Texts.h"
#include <algorithm>
#include <gtest/gtest.h>
#include <string>

TEST(HeroInfoState, OpenHeroInfoPops) {
    auto session = MakeTestSession();
    game_states::HeroInfoState state(session);

    auto result = state.OnAction(ui::Action::OpenHeroInfo);

    if (!result.has_value()) {
        FAIL() << "expected OpenHeroInfo to produce a GameState::Result";
        return;
    }
    EXPECT_EQ(result.value().type, game_states::GameState::Result::Type::Pop);
}

TEST(HeroInfoState, CancelPops) {
    auto session = MakeTestSession();
    game_states::HeroInfoState state(session);

    auto result = state.OnAction(ui::Action::Cancel);

    if (!result.has_value()) {
        FAIL() << "expected Cancel to produce a GameState::Result";
        return;
    }
    EXPECT_EQ(result.value().type, game_states::GameState::Result::Type::Pop);
}

TEST(HeroInfoState, UnrelatedActionIsIgnored) {
    auto session = MakeTestSession();
    game_states::HeroInfoState state(session);

    auto result = state.OnAction(ui::Action::MoveNorth);

    EXPECT_FALSE(result.has_value());
}

namespace {
bool HasRow(const ui::RenderScene &scene, const std::string &text) {
    return std::ranges::any_of(scene.texts,
                               [&text](const ui::RenderText &row) { return row.text == text; });
}
} // namespace

TEST(HeroInfoState, PanelPrintsAttributesAndSkillsFromWorldData) {
    auto session = MakeTestSession();
    const auto hero_id = session.registry.entities.CreateEntity();
    session.registry.Storage<component::PlayerTag>().Add(hero_id, {});
    session.registry.Storage<component::NameComponent>().Add(hero_id, {"Bob"});
    session.registry.Storage<component::AttributesComponent>().Add(
        hero_id, {.chars = {11, 12, 13, 14, 15, 16}});
    session.registry.Storage<hero::HeroSkills>().Add(hero_id, {.skills = {20, 21, 22, 23, 24}});
    session.registry.Storage<hero::HeroExperience>().Add(
        hero_id, {.xp = 42, .level = 3, .hclass = hero::HClass::Mage});

    const game_states::HeroInfoState state(session);
    ui::RenderScene scene;
    state.BuildScene(scene);

    EXPECT_TRUE(HasRow(scene, "name: Bob"));
    EXPECT_TRUE(HasRow(scene, "Mage, level 3 (xp 42)"));
    EXPECT_TRUE(HasRow(scene, "Chars:"));
    EXPECT_TRUE(HasRow(scene, texts::CharNames().at(0) + ": 11"));
    EXPECT_TRUE(HasRow(scene, texts::CharNames().at(5) + ": 16"));
    EXPECT_TRUE(HasRow(scene, "Skills:"));
    EXPECT_TRUE(HasRow(scene, texts::SkillNames().at(0) + ": 20"));
    EXPECT_TRUE(HasRow(scene, texts::SkillNames().at(4) + ": 24"));
}

TEST(HeroInfoState, PanelOmitsSectionsWhoseComponentsAreMissing) {
    auto session = MakeTestSession();
    const auto hero_id = session.registry.entities.CreateEntity();
    session.registry.Storage<component::PlayerTag>().Add(hero_id, {});
    session.registry.Storage<component::NameComponent>().Add(hero_id, {"Bob"});

    const game_states::HeroInfoState state(session);
    ui::RenderScene scene;
    state.BuildScene(scene);

    EXPECT_TRUE(HasRow(scene, "name: Bob"));
    EXPECT_FALSE(HasRow(scene, "Chars:"));
    EXPECT_FALSE(HasRow(scene, "Skills:"));
}
