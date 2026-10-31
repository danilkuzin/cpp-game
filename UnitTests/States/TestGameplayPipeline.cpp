#include "Bindings.h"
#include "Components.h"
#include "EntityStorage.h"
#include "GameplayState.h"
#include "Location.h"
#include "Registry.h"
#include "Session.h"
#include "Support/Fixtures.h"
#include "Support/MakeSession.h"
#include "Texts.h"
#include "Vector2Int.h"
#include <gtest/gtest.h>
#include <string>

namespace {
entity::EntityId MakePipelineHero(domain::Registry &registry, Vector2Int coord) {
    auto id = MakeHeroAt(registry, coord);
    registry.Storage<component::AttributesComponent>().Add(id, WithStr(20));
    return id;
}

entity::EntityId MakeStationaryRat(domain::Registry &registry, Vector2Int coord) {
    return MakeNamedMonsterAt(registry, coord, /*hp=*/1, "Rat");
}

void StepEast(game_states::GameplayState &state, const domain::Registry &registry) {
    state.OnAction(ui::Action::MoveEast);
    EXPECT_NO_THROW(state.Update());
    ExpectNoIntentsSurvived(registry);
}
} // namespace

TEST(GameplayPipeline, HeroWalksThreeStepsBumpsRatAndRatDies) {
    auto session = MakeTestSession();
    const auto hero_id = MakePipelineHero(session.registry, {.x = 5, .y = 5});
    auto rat = MakeStationaryRat(session.registry, {.x = 8, .y = 5});
    game_states::GameplayState state(session);

    for (int step = 0; step < 3; step++) {
        StepEast(state, session.registry);
    }

    const auto &hero_loc = session.registry.GetRef<Location>(hero_id);
    EXPECT_EQ(hero_loc.coord, (Vector2Int{.x = 7, .y = 5}));

    EXPECT_EQ(session.registry.Storage<component::HealthComponent>().Get(rat), nullptr);
    EXPECT_EQ(session.registry.Storage<Location>().Get(rat), nullptr);

    bool logged_kill = false;
    for (const auto &line : session.runtime.log) {
        logged_kill = logged_kill || (line.contains(texts::StrMonKill()) && line.contains("Hero") &&
                                      line.contains("Rat"));
    }
    EXPECT_TRUE(logged_kill);
}

TEST(GameplayPipeline, ANonTurnEventDoesNotRefreshStats) {
    auto session = MakeTestSession();
    MakePipelineHero(session.registry, {.x = 5, .y = 5});
    game_states::GameplayState state(session);

    state.OnAction(ui::Action::Cancel);
    EXPECT_NO_THROW(state.Update());

    EXPECT_TRUE(session.stats.hero_name.empty());
    ExpectNoIntentsSurvived(session.registry);
}
