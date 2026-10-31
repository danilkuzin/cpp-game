cmake_minimum_required(VERSION 3.28)
project(cppgame VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)
option(ENABLE_SANITIZERS "Enable sanitizers" OFF)

add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_23)

if (MSVC)
    target_compile_options(project_options INTERFACE /W4 /WX)
else()
    target_compile_options(project_options INTERFACE
        -Wall -Wextra -Wpedantic -Wimplicit-fallthrough -Werror)
endif()

if (ENABLE_SANITIZERS AND NOT MSVC)
    target_compile_options(project_options INTERFACE
        -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer)
    target_link_options(project_options INTERFACE
        -fsanitize=address,undefined)
endif()

if (ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE clang-tidy REQUIRED)
endif()

function(apply_project_options target)
    target_link_libraries(${target} PRIVATE project_options)
    if (ENABLE_CLANG_TIDY)
        set_target_properties(${target} PROPERTIES
            CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
    endif()
endfunction()

include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.17.0
        GIT_SHALLOW TRUE
        SYSTEM
        EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(
        SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG release-3.4.12
        GIT_SHALLOW TRUE
        SYSTEM
        EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(SDL3)

FetchContent_Declare(
        SDL_ttf
        GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
        GIT_TAG release-3.2.2
        GIT_SHALLOW TRUE
        SYSTEM
        EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(SDL_ttf)

FetchContent_Declare(
        json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.12.0
        GIT_SHALLOW TRUE
        SYSTEM
        EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(json)

enable_testing()
include(GoogleTest)

add_subdirectory(Texts)
add_subdirectory(Core)
apply_project_options(core)
add_subdirectory(Domain)
apply_project_options(domain)
add_subdirectory(Ui)
apply_project_options(Ui)
add_subdirectory(Graphics)
apply_project_options(graphics)
add_subdirectory(GameStates)
apply_project_options(game_states)
add_subdirectory(Game)
apply_project_options(game)
add_subdirectory(UnitTests)
apply_project_options(UnitTests)


add_custom_target(copy_assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${PROJECT_SOURCE_DIR}/assets" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${PROJECT_SOURCE_DIR}/Data" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Data"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${PROJECT_SOURCE_DIR}/config.json" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/config.json"
    COMMENT "Copying assets, Data and config.json next to the built executables"
    VERBATIM
)
add_dependencies(game copy_assets)
add_dependencies(UnitTests copy_assets)
