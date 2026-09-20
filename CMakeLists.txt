cmake_minimum_required(VERSION 3.20)
project(ECSEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ECSEngine_ENABLE_RUNTIME "Enable SFML-dependent engine runtime targets" ON)
option(ECSEngine_COMPILE_ALL_FOR_COVERAGE "Compile all engine source files into a coverage-only object target" OFF)

# Remove SFML_STATIC - using dynamic SFML now
add_compile_definitions(NOMINMAX WIN32_LEAN_AND_MEAN)

# Source files
file(GLOB ROOT_SOURCES        CONFIGURE_DEPENDS "*.cpp")
file(GLOB BASE_SOURCES        CONFIGURE_DEPENDS "Base/src/*.cpp")
file(GLOB MANAGER_SOURCES     CONFIGURE_DEPENDS "Managers/src/*.cpp")
file(GLOB SYSTEM_SOURCES      CONFIGURE_DEPENDS "Systems/src/*.cpp")
file(GLOB DATA_SOURCES        CONFIGURE_DEPENDS "DataClasses/src/*.cpp")
file(GLOB DIALOG_SOURCES       CONFIGURE_DEPENDS "Dialogs/src/*.cpp")
file(GLOB TOOL_SOURCES        CONFIGURE_DEPENDS "Tools/src/*.cpp")
file(GLOB HOT_RELOAD_SOURCES  CONFIGURE_DEPENDS "HotReloading/src/*.cpp")
file(GLOB COLLISION_SOURCES   CONFIGURE_DEPENDS "Collision/*.cpp")
file(GLOB UI_SOURCES          CONFIGURE_DEPENDS "UI/*.cpp")
file(GLOB COMPONENTS_SOURCES  CONFIGURE_DEPENDS "Components/src/*.cpp")
file(GLOB PROJECT_EXPLORER_SOURCES  CONFIGURE_DEPENDS "ProjectExplorer/src/*.cpp")
file(GLOB SERIALIZATION_SOURCES  CONFIGURE_DEPENDS "Serialization/src/*.cpp")
list(REMOVE_ITEM ROOT_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")

set(IMGUI_SOURCES
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_tables.cpp
    imgui/imgui-SFML.cpp
)

add_library(ECSEngineCore SHARED
    ${ROOT_SOURCES}
    ${BASE_SOURCES}
    ${MANAGER_SOURCES}
    ${SYSTEM_SOURCES}
    ${DATA_SOURCES}
    ${TOOL_SOURCES}
    ${HOT_RELOAD_SOURCES}
    ${COLLISION_SOURCES}
    ${COMPONENTS_SOURCES}
    ${PROJECT_EXPLORER_SOURCES}
    ${UI_SOURCES}
    ${IMGUI_SOURCES}
    ${DIALOG_SOURCES}
    ${SERIALIZATION_SOURCES}
)

set_target_properties(ECSEngineCore PROPERTIES 
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)

add_executable(ECSEngine main.cpp)

# Includes
target_include_directories(ECSEngineCore PUBLIC
    .
    Base/include/
    Managers/include/
    Systems/include/
    DataClasses/include/
    Tools/include/
    Dialogs/include/
    Dialogs/include/base/
    HotReloading/include/
    ProjectExplorer/include/
    Serialization/include/
    Components/
    imgui/
    include/
    Collision/
    Commands/
    UI/
)

target_include_directories(ECSEngine PRIVATE .)

if(ECSEngine_COMPILE_ALL_FOR_COVERAGE)
    add_library(ECSEngineCoverageObjects OBJECT
        ${ROOT_SOURCES}
        ${BASE_SOURCES}
        ${MANAGER_SOURCES}
        ${SYSTEM_SOURCES}
        ${DATA_SOURCES}
        ${TOOL_SOURCES}
        ${HOT_RELOAD_SOURCES}
        ${COLLISION_SOURCES}
        ${COMPONENTS_SOURCES}
        ${PROJECT_EXPLORER_SOURCES}
        ${UI_SOURCES}
        ${IMGUI_SOURCES}
        ${DIALOG_SOURCES}
        ${SERIALIZATION_SOURCES}
    )

    target_include_directories(ECSEngineCoverageObjects PRIVATE
        .
        Base/include/
        Managers/include/
        Systems/include/
        DataClasses/include/
        Tools/include/
        Dialogs/include/
        Dialogs/include/base/
        HotReloading/include/
        ProjectExplorer/include/
        Serialization/include/
        Components/
        imgui/
        include/
        Collision/
        Commands/
        UI/
    )
endif()

if(ECSEngine_ENABLE_RUNTIME)
    # Dynamic SFML
    set(SFML_STATIC_LIBRARIES FALSE)
    set(SFML_DIR "${CMAKE_SOURCE_DIR}/lib/cmake/SFML")
    find_package(SFML 2.6 COMPONENTS graphics window system audio REQUIRED)

    target_link_directories(ECSEngineCore PUBLIC "${CMAKE_SOURCE_DIR}/lib")

    # Link dynamic SFML libs (no -s suffix = dynamic, -d = debug)
    target_link_libraries(ECSEngineCore PUBLIC
        sfml-graphics-d
        sfml-window-d
        sfml-system-d
        sfml-audio-d
        opengl32
        winmm
        gdi32
    )
endif()

target_link_libraries(ECSEngine PRIVATE ECSEngineCore)

include(CTest)

if(BUILD_TESTING)

    # =========================
    # Unit Tests
    # =========================

    add_executable(ECSEngineUnitTests
        Tests/TestSuite.cpp
        Base/src/ComponentTypeRegistry.cpp
        DataClasses/src/AssetReference.cpp
        DataClasses/src/Prefab.cpp
        Systems/src/EventSystem.cpp
        Systems/src/SystemManager.cpp
    )

    target_include_directories(ECSEngineUnitTests PRIVATE
        Base/include/
        DataClasses/include/
        Commands/
        Serialization/include/
        Systems/include/
        Tools/include/
    )

    target_link_libraries(ECSEngineUnitTests PRIVATE
        ECSEngineCore
    )

    add_custom_command(TARGET ECSEngineUnitTests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:ECSEngineCore>"
            "$<TARGET_FILE_DIR:ECSEngineUnitTests>"
    )

    function(add_engine_unit_test test_name)
        add_test(
            NAME ${test_name}
            COMMAND ECSEngineUnitTests ${test_name}
        )
    endfunction()

    add_engine_unit_test(Registry.RegistersAndFindsAName)
    add_engine_unit_test(Registry.UnknownNamesAreInvalid)
    add_engine_unit_test(Registry.AssociatesAndUnregistersTypes)
    add_engine_unit_test(Vector.ArithmeticAndComparison)
    add_engine_unit_test(Vector.MathProducesExpectedResults)
    add_engine_unit_test(Vector.Vector3AndVector4SupportCoreOperations)
    add_engine_unit_test(Guid.GenerationUsesRfc4122VersionFourFormat)
    add_engine_unit_test(CommandHistory.ExecutesUndoesAndRedoes)
    add_engine_unit_test(Registry.StopsAtItsCapacity)
    add_engine_unit_test(Events.DispatchesAndUnsubscribesSafely)
    add_engine_unit_test(Events.KeepsEventChannelsIndependent)
    add_engine_unit_test(Serialization.PreservesEntityAndComponentData)
    add_engine_unit_test(Logger.DeliversLevelAndMessageToCallback)
    add_engine_unit_test(Clipboard.CopiesAndClearsSerializedEntities)
    add_engine_unit_test(Systems.UpdatesRegisteredSystems)
    add_engine_unit_test(Assets.MaintainsReferenceLifecycleAndPaths)

    if(ECSEngine_ENABLE_RUNTIME)
        add_executable(ECSEngineIntegrationTests
            Tests/IntegrationSuite.cpp
        )

        target_link_libraries(ECSEngineIntegrationTests PRIVATE ECSEngineCore)

        add_test(
            NAME Integration.EntityManagerLifecycle
            COMMAND ECSEngineIntegrationTests EntityManagerLifecycle
        )
        add_test(
            NAME Integration.EntityHierarchyAndTransforms
            COMMAND ECSEngineIntegrationTests EntityHierarchyAndTransforms
        )
        add_test(
            NAME Integration.EntityActiveHierarchy
            COMMAND ECSEngineIntegrationTests EntityActiveHierarchy
        )
        add_test(
            NAME Integration.EntityManagerRemovesPendingEntities
            COMMAND ECSEngineIntegrationTests EntityManagerRemovesPendingEntities
        )
        add_test(
            NAME Integration.EntityManagerTracksCollisionRecords
            COMMAND ECSEngineIntegrationTests EntityManagerTracksCollisionRecords
        )
        add_test(
            NAME Integration.EntitySerializesComponentFields
            COMMAND ECSEngineIntegrationTests EntitySerializesComponentFields
        )
        add_test(
            NAME Integration.EntityComponentLifecycle
            COMMAND ECSEngineIntegrationTests EntityComponentLifecycle
        )
        add_test(
            NAME Integration.EntityManagerClearsInspectorState
            COMMAND ECSEngineIntegrationTests EntityManagerClearsInspectorState
        )
        add_test(
            NAME Integration.EntityManagerSerializesEntitySet
            COMMAND ECSEngineIntegrationTests EntityManagerSerializesEntitySet
        )

        add_custom_command(TARGET ECSEngineIntegrationTests POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<TARGET_FILE:ECSEngineCore>"
                "$<TARGET_FILE_DIR:ECSEngineIntegrationTests>"
        )
    endif()

endif()

if(ECSEngine_ENABLE_RUNTIME)
    # Copy SFML DLLs next to exe so they're found at runtime
    set(SFML_BIN_DIR "${CMAKE_SOURCE_DIR}/bin")
    add_custom_command(TARGET ECSEngine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SFML_BIN_DIR}/sfml-graphics-d-2.dll"
            "${SFML_BIN_DIR}/sfml-window-d-2.dll"
            "${SFML_BIN_DIR}/sfml-system-d-2.dll"
            "${SFML_BIN_DIR}/sfml-audio-d-2.dll"
            "${SFML_BIN_DIR}/OpenAL32.dll"
            $<TARGET_FILE_DIR:ECSEngine>
    )
endif()