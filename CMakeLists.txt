cmake_minimum_required(VERSION 3.20)
project(ECSEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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
    ${UI_SOURCES}
    ${IMGUI_SOURCES}
    ${DIALOG_SOURCES}
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
    Components/
    imgui/
    include/
    Collision/
    Commands/
    UI/
)

target_include_directories(ECSEngine PRIVATE .)

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

target_link_libraries(ECSEngine PRIVATE ECSEngineCore)

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