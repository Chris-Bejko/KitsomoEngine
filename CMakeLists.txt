cmake_minimum_required(VERSION 3.20)
project(ECSEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Definitions
add_compile_definitions(SFML_STATIC)
add_compile_definitions(NOMINMAX WIN32_LEAN_AND_MEAN)

# Source files in root
file(GLOB SOURCES "*.cpp")
file(GLOB COLLISION_SOURCES "Collision/*.cpp")
file(GLOB UI_SOURCES "UI/*.cpp")

# Components folder
file(GLOB COMPONENTS_SOURCES "Components/*.cpp")

# ImGui sources
set(IMGUI_SOURCES
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_tables.cpp
    imgui/imgui-SFML.cpp
)

add_executable(ECSEngine
    ${SOURCES}
    ${COLLISION_SOURCES}
    ${COMPONENTS_SOURCES}
    ${UI_SOURCES}
    ${IMGUI_SOURCES}
)

# Includes
target_include_directories(ECSEngine PRIVATE
    .
    imgui/
    include/
    Collision/  
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