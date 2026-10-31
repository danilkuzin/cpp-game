add_library(Ui STATIC
        Bindings.cpp
        Camera.cpp
        CharacterGenerationSceneBuilder.cpp
        Coordinates.cpp
        EventFormat.cpp
        GameSceneBuilder.cpp
        HeroInfoSceneBuilder.cpp
        MessageLog.cpp
        PanelLayout.cpp
        RenderScene.cpp
        SidebarPanel.cpp
        WorldSceneBuilder.cpp
)
target_include_directories(Ui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(Ui PUBLIC domain PRIVATE texts)
