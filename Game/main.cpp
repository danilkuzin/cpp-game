#include "CharacterGenState.h"
#include "DataLoader.h"
#include "Game.h"
#include "RectInt.h"
#include "Renderer.h"
#include "Roll.h"
#include "Session.h"
#include "WorldGenerator.h"
#include <exception>
#include <iostream>
#include <memory>
#include <random>
#include <utility>

namespace {
game_states::Session MakeContext() {
    auto config = dataloader::DataLoader::LoadConfig("config.json");
    auto view = dataloader::DataLoader::LoadViewConfig("config.json");
    auto db = dataloader::DataLoader::LoadDatabase("Data");

    game_states::Session session{
        .config = config,
        .registry = domain::Registry{},
        .runtime =
            game_states::RuntimeState{.camera = ui::Camera{RectInt{.x = view.camera_width,
                                                                   .y = view.camera_height,
                                                                   .width = view.camera_width,
                                                                   .height = view.camera_height},
                                                           view.scroll_delta},
                                      .log = {}},
        .db = db,
        .stats = {}};

    return session;
}
} // namespace

int main() {
    try {
        auto session = MakeContext();

        const unsigned int seed = session.config.seed.value_or(std::random_device{}());
        roll::Seed(seed);
        std::cout << "seed: " << seed << "\n";

        auto renderer = graphics::Renderer();
        auto initial_state = std::make_unique<game_states::CharacterGenState>(session);
        game::Game game(renderer, std::move(initial_state));

        domain::WorldGenerator::GenerateMaps(session.registry, session.config.map_config);
        domain::WorldGenerator::GenerateTileFeatures(session.registry);
        domain::WorldGenerator::GenerateItems(session.registry, session.db);
        domain::WorldGenerator::GenerateMonsters(session.registry, session.db,
                                                 session.config.num_monsters);
        game.Run();
    } catch (const std::exception &e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
