add_library(core STATIC
        Location.cpp
        RectInt.cpp
        Vector2Int.cpp)
target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(core PUBLIC cxx_std_23)