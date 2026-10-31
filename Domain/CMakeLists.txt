add_library(domain STATIC
        Combat.cpp
        DataLoader.cpp
        EffectSystems.cpp
        EntityFactory.cpp
        EntityStorage.cpp
        GameDatabase.cpp
        Inventory.cpp
        Hero.cpp
        Leveling.cpp
        Map.cpp
        RegistryQueries.cpp
        Roll.cpp
        Systems.cpp
        Terrain.cpp
        Trade.cpp
        WorldGenerator.cpp
)
target_include_directories(domain PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(domain PUBLIC core  PRIVATE nlohmann_json::nlohmann_json texts)
