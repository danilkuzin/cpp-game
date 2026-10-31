add_library(game_states
        CharacterGenState.cpp
        GameOverState.cpp
        GameplayState.cpp
        GameState.cpp
        HeroInfoState.cpp
        InventoryState.cpp
        Session.cpp
        StateStack.cpp
        TradeState.cpp
)
target_include_directories(game_states PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(game_states PUBLIC domain Ui PRIVATE texts)
