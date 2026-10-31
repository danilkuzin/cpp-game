add_library(graphics STATIC
        InputHandler.cpp
        Renderer.cpp
        TextCache.cpp
        TextureManager.cpp
)
target_include_directories(graphics PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(graphics PUBLIC domain Ui SDL3::SDL3 SDL3_ttf::SDL3_ttf nlohmann_json::nlohmann_json PRIVATE texts)
