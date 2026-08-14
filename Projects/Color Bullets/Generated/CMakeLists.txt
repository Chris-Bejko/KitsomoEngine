cmake_minimum_required(VERSION 3.20)
project(GameScripts)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT DEFINED ENGINE_ROOT)
    message(FATAL_ERROR "ENGINE_ROOT is required")
endif()
if(NOT DEFINED PROJECT_ROOT)
    message(FATAL_ERROR "PROJECT_ROOT is required")
endif()
get_filename_component(ENGINE_ROOT "${ENGINE_ROOT}" ABSOLUTE)
get_filename_component(PROJECT_ROOT "${PROJECT_ROOT}" ABSOLUTE)

add_compile_definitions(NOMINMAX WIN32_LEAN_AND_MEAN ECS_PROJECT_MODULE_BUILD)

add_library(GameScripts SHARED
    "${PROJECT_ROOT}/Generated/RegisterProjectComponents.cpp"
    "${PROJECT_ROOT}/Assets/Scripts/Bullet.cpp"
    "${PROJECT_ROOT}/Assets/Scripts/FloorSquare.cpp"
    "${PROJECT_ROOT}/Assets/Scripts/GameManager.cpp"
    "${PROJECT_ROOT}/Assets/Scripts/Player.cpp"
)

if(NOT DEFINED BUILD_GENERATION)
    set(BUILD_GENERATION 0)
endif()

set_target_properties(GameScripts PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PROJECT_ROOT}/build/Debug"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG "${PROJECT_ROOT}/build/Debug"
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${PROJECT_ROOT}/build/Debug"
    PDB_OUTPUT_DIRECTORY_DEBUG "${PROJECT_ROOT}/build/PDB"
    PDB_NAME_DEBUG "GameScripts_${BUILD_GENERATION}"
    COMPILE_PDB_OUTPUT_DIRECTORY_DEBUG "${PROJECT_ROOT}/build/PDB"
    COMPILE_PDB_NAME_DEBUG "GameScripts_${BUILD_GENERATION}"
)

target_include_directories(GameScripts PRIVATE
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
    "${PROJECT_ROOT}/Assets/Scripts"
)

set(SFML_STATIC_LIBRARIES FALSE)
set(SFML_DIR "${ENGINE_ROOT}/lib/cmake/SFML")
find_package(SFML 2.6 COMPONENTS graphics window system audio REQUIRED)

target_link_directories(GameScripts PRIVATE "${ENGINE_ROOT}/build/Debug" "${ENGINE_ROOT}/lib")

target_link_libraries(GameScripts PRIVATE
    ECSEngineCore
    sfml-graphics
    sfml-window
    sfml-system
    sfml-audio
)

