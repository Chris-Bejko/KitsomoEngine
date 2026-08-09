cmake_minimum_required(VERSION 3.20)
project(ECSEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Definitions
add_compile_definitions(SFML_STATIC)
add_compile_definitions(NOMINMAX WIN32_LEAN_AND_MEAN)

# Source files
file(GLOB ROOT_SOURCES CONFIGURE_DEPENDS "*.cpp")
file(GLOB BASE_SOURCES CONFIGURE_DEPENDS "Base/src/*.cpp")
file(GLOB MANAGER_SOURCES CONFIGURE_DEPENDS "Managers/src/*.cpp")
file(GLOB SYSTEM_SOURCES CONFIGURE_DEPENDS "Systems/src/*.cpp")
file(GLOB DATA_SOURCES CONFIGURE_DEPENDS "DataClasses/src/*.cpp")
file(GLOB TOOL_SOURCES CONFIGURE_DEPENDS "Tools/src/*.cpp")
file(GLOB COLLISION_SOURCES CONFIGURE_DEPENDS "Collision/*.cpp")
file(GLOB UI_SOURCES CONFIGURE_DEPENDS "UI/*.cpp")
file(GLOB COMPONENTS_SOURCES CONFIGURE_DEPENDS "Components/src/*.cpp")
# file(GLOB PROJECT_SCRIPT_SOURCES CONFIGURE_DEPENDS "Projects/Color Bullets/Scripts/*.cpp")1

# ImGui sources
set(IMGUI_SOURCES
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_tables.cpp
    imgui/imgui-SFML.cpp
)

add_executable(ECSEngine
    ${ROOT_SOURCES}
    ${BASE_SOURCES}
    ${MANAGER_SOURCES}
    ${SYSTEM_SOURCES}
    ${DATA_SOURCES}
    ${TOOL_SOURCES}
    ${COLLISION_SOURCES}
    ${COMPONENTS_SOURCES}
    ${PROJECT_SCRIPT_SOURCES}
    ${UI_SOURCES}
    ${IMGUI_SOURCES}
)

# Includes
target_include_directories(ECSEngine PRIVATE
    .
    Base/include/
    Managers/include/
    Systems/include/
    DataClasses/include/
    Tools/include/
    Components/
    Projects/Color Bullets/Scripts/
    imgui/
    include/
    Collision/
    Commands/
    UI/
)

# SFML
set(SFML_STATIC_LIBRARIES TRUE)
set(SFML_DIR "${CMAKE_SOURCE_DIR}/lib/cmake/SFML")
find_package(SFML 2.6 COMPONENTS graphics window system audio REQUIRED)
add_custom_command(TARGET ECSEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/bin/OpenAL32.dll"
    $<TARGET_FILE_DIR:ECSEngine>
)
target_link_directories(ECSEngine PRIVATE "${CMAKE_SOURCE_DIR}/lib")

target_link_libraries(ECSEngine PRIVATE
    sfml-graphics-s-d
    sfml-window-s-d
    sfml-system-s-d
    sfml-audio-s-d
    opengl32
    winmm
    gdi32
    freetype
    flac
    vorbis
    vorbisenc
    vorbisfile
    ogg
    openal32
)