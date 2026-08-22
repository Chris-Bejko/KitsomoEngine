cmake_minimum_required(VERSION 3.20)
project(MyGame)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(ENGINE_ROOT "C:/Users/kitso/Documents/CPP Game/CPP PRoject/ECSEngine")
set(EXPORT_ROOT "C:/Users/kitso/Documents/CPP Game/CPP PRoject/ECSEngine/Export")

add_compile_definitions(
    NOMINMAX
    WIN32_LEAN_AND_MEAN
)

add_executable(MyGame
    "${EXPORT_ROOT}/Generated/RuntimeMain.cpp"
)

target_include_directories(MyGame PRIVATE
    "${ENGINE_ROOT}"
    "${ENGINE_ROOT}/Base/include"
    "${ENGINE_ROOT}/Managers/include"
    "${ENGINE_ROOT}/Systems/include"
    "${ENGINE_ROOT}/DataClasses/include"
    "${ENGINE_ROOT}/Tools/include"
    "${ENGINE_ROOT}/Collision"
    "${ENGINE_ROOT}/Components"
    "${ENGINE_ROOT}/UI"
    "${ENGINE_ROOT}/Commands"
    "${ENGINE_ROOT}/include"
    "${ENGINE_ROOT}/imgui"
)

target_link_directories(MyGame PRIVATE
    "${ENGINE_ROOT}/build/Debug"
    "${ENGINE_ROOT}/lib"
)

set(SFML_STATIC_LIBRARIES FALSE)
set(SFML_DIR "${ENGINE_ROOT}/lib/cmake/SFML")
find_package(SFML 2.6 COMPONENTS graphics window system audio REQUIRED)

target_link_libraries(MyGame PRIVATE
    ECSEngineCore
    sfml-graphics-d
    sfml-window-d
    sfml-system-d
    sfml-audio-d
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

set_target_properties(MyGame PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${EXPORT_ROOT}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${EXPORT_ROOT}"
)
