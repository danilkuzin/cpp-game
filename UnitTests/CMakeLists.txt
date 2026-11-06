add_executable(UnitTests
        Core/TestRectInt.cpp
        Core/TestVector2Int.cpp
        Domain/TestAISystem.cpp
        Domain/TestCollisionSystem.cpp
        Domain/TestCombat.cpp
        Domain/TestDamageSystem.cpp
        Domain/TestDataLoader.cpp
        Domain/TestDeathSystem.cpp
        Domain/TestEntitiesAt.cpp
        Domain/TestHealingSource.cpp
        Domain/TestHealSystem.cpp
        Domain/TestHero.cpp
        Domain/TestInventory.cpp
        Domain/TestLeveling.cpp
        Domain/TestMap.cpp
        Domain/TestMeleeSystem.cpp
        Domain/TestMonster.cpp
        Domain/TestMovementSystem.cpp
        Domain/TestNavigation.cpp
        Domain/TestRangedSystem.cpp
        Domain/TestRegistry.cpp
        Domain/TestRoll.cpp
        Domain/TestSpellSystem.cpp
        Domain/TestTerrain.cpp
        Domain/TestTrade.cpp
        Domain/TestTrapEncounter.cpp
        Domain/TestWorldGenerator.cpp
        States/TestCharacterGenState.cpp
        States/TestGameOverState.cpp
        States/TestGameplayInput.cpp
        States/TestGameplayPipeline.cpp
        States/TestHeroInfoState.cpp
        States/TestInventoryState.cpp
        States/TestStateStack.cpp
        States/TestTargeting.cpp
        States/TestTradeState.cpp
        Support/MakeRegistry.cpp
        Support/MakeSession.cpp
        Support/TestData.cpp
        Ui/TestBindings.cpp
        Ui/TestCamera.cpp
        Ui/TestEventFormat.cpp
        Ui/TestHitTest.cpp
        Ui/TestMessageLog.cpp
        Ui/TestOutline.cpp
        Ui/TestPanelLayout.cpp
        Ui/TestPanelRects.cpp
        Ui/TestSceneBuilder.cpp
        Ui/TestScreenToTile.cpp
        Ui/TestSidebarPanel.cpp
)
target_include_directories(UnitTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(UnitTests PRIVATE GTest::gtest_main domain core game_states texts)

# Four gtest_discover_tests calls over the same binary, split by TEST_FILTER, so `ctest -L <label>`
# runs one subdirectory's tests while everything lives in one executable. Suite names, not source
# paths, drive the split — keep these filters in sync with Core/, States/ and Ui/.
set(UnitTests_CORE_FILTER "Vector2Int.*:RectInt.*")
set(UnitTests_STATES_FILTER
    "CharacterGenState.*:GameOverState.*:GameplayInput.*:GameplayPipeline.*:HeroInfoState.*:InventoryState.*:StateStack.*:Targeting.*:TradeState.*"
)
set(UnitTests_UI_FILTER
    "Bindings.*:Camera.*:Coordinates.*:CountMonstersSlain.*:EventFormat.*:FormatEventResult.*:GameSceneBuilder.*:HitTest.*:MessageLog.*:OutlineEdges.*:PanelLayout.*:PanelRects.*:ScreenToTile.*:SidebarPanel.*:WorldSceneBuilder.*"
)

gtest_discover_tests(UnitTests DISCOVERY_MODE PRE_TEST
    WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    TEST_FILTER "${UnitTests_CORE_FILTER}"
    PROPERTIES LABELS "core"
)
gtest_discover_tests(UnitTests DISCOVERY_MODE PRE_TEST
    WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    TEST_FILTER "${UnitTests_STATES_FILTER}"
    PROPERTIES LABELS "states"
)
gtest_discover_tests(UnitTests DISCOVERY_MODE PRE_TEST
    WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    TEST_FILTER "${UnitTests_UI_FILTER}"
    PROPERTIES LABELS "ui"
)
gtest_discover_tests(UnitTests DISCOVERY_MODE PRE_TEST
    WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    TEST_FILTER "*-${UnitTests_CORE_FILTER}:${UnitTests_STATES_FILTER}:${UnitTests_UI_FILTER}"
    PROPERTIES LABELS "domain"
)
