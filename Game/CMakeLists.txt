add_executable(game
        Game.cpp
        main.cpp
)
target_include_directories(game PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(game PRIVATE graphics game_states)
