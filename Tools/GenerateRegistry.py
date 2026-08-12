import re
import sys
from pathlib import Path


PROJECT_ROOT = Path(sys.argv[1]).resolve()
ENGINE_ROOT = Path(sys.argv[2]).resolve()
SCRIPTS_DIR = PROJECT_ROOT / "Assets" / "Scripts"
GENERATED_DIR = PROJECT_ROOT / "Generated"
OUTPUT_CPP = GENERATED_DIR / "RegisterProjectComponents.cpp"
OUTPUT_CMAKE = GENERATED_DIR / "CMakeLists.txt"


def find_serializable_scripts(folder: Path):
    scripts = []

    pattern = re.compile(
        r'class\s+(\w+)\s*:\s*public\s+SerializableScript'
    )

    if not folder.exists():
        return scripts

    for header in sorted(folder.glob("*.h")):
        content = header.read_text(encoding="utf-8")

        for match in pattern.findall(content):
            cpp = header.with_suffix(".cpp")

            scripts.append({
                "name": match,
                "header": header.name,
                "cpp": cpp.name if cpp.exists() else None,
            })

    return scripts

def generate_registry(scripts):
    lines = []
    lines.append("// AUTO-GENERATED - DO NOT EDIT")
    lines.append('#include "HotReloading/include/ProjectComponentRegistration.h"')
    lines.append('#include "HotReloading/include/ProjectModuleAPI.h"')
    lines.append("")

    for script in scripts:
        lines.append(f'#include "{script["header"]}"')

    lines.append("")
    lines.append('extern "C" __declspec(dllexport) void RegisterProjectComponents(RegisterProjectComponentFn registerFn)')
    lines.append("{")
    for script in scripts:
        lines.append(f'    RegisterProjectComponent<{script["name"]}>(registerFn, "{script["name"]}");')
    lines.append("}")

    OUTPUT_CPP.write_text("\n".join(lines) + "\n", encoding="utf-8")


def generate_cmake(scripts):
    lines = []
    lines.append("cmake_minimum_required(VERSION 3.20)")
    lines.append("project(GameScripts)")
    lines.append("")
    lines.append("set(CMAKE_CXX_STANDARD 17)")
    lines.append("set(CMAKE_CXX_STANDARD_REQUIRED ON)")
    lines.append("set(CMAKE_CXX_EXTENSIONS OFF)")
    lines.append("")

    lines.append("if(NOT DEFINED ENGINE_ROOT)")
    lines.append('    message(FATAL_ERROR "ENGINE_ROOT is required")')
    lines.append("endif()")

    lines.append("if(NOT DEFINED PROJECT_ROOT)")
    lines.append('    message(FATAL_ERROR "PROJECT_ROOT is required")')
    lines.append("endif()")

    lines.append('get_filename_component(ENGINE_ROOT "${ENGINE_ROOT}" ABSOLUTE)')
    lines.append('get_filename_component(PROJECT_ROOT "${PROJECT_ROOT}" ABSOLUTE)')
    lines.append("")

    lines.append(
        "add_compile_definitions("
        "SFML_STATIC "
        "NOMINMAX "
        "WIN32_LEAN_AND_MEAN "
        "ECS_PROJECT_MODULE_BUILD"
        ")"
    )
    lines.append("")

    # ------------------------------------------------------------
    # GameScripts source files
    # ------------------------------------------------------------

    lines.append("add_library(GameScripts SHARED")
    lines.append(
        '    "${PROJECT_ROOT}/Generated/RegisterProjectComponents.cpp"'
    )

    for script in scripts:
        if script["cpp"] is not None:
            lines.append(
                f'    "${{PROJECT_ROOT}}/Assets/Scripts/{script["cpp"]}"'
            )

    lines.append(")")
    lines.append("")
    lines.append("if(NOT DEFINED BUILD_GENERATION)")
    lines.append("    set(BUILD_GENERATION 0)")
    lines.append("endif()")
    lines.append("")

    lines.append("set_target_properties(GameScripts PROPERTIES")
    lines.append('    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PROJECT_ROOT}/build/Debug"')
    lines.append('    LIBRARY_OUTPUT_DIRECTORY_DEBUG "${PROJECT_ROOT}/build/Debug"')
    lines.append('    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${PROJECT_ROOT}/build/Debug"')
    lines.append('    PDB_OUTPUT_DIRECTORY_DEBUG "${PROJECT_ROOT}/build/PDB"')
    lines.append('    PDB_NAME_DEBUG "GameScripts_${BUILD_GENERATION}"')
    lines.append('    COMPILE_PDB_OUTPUT_DIRECTORY_DEBUG "${PROJECT_ROOT}/build/PDB"')
    lines.append('    COMPILE_PDB_NAME_DEBUG "GameScripts_${BUILD_GENERATION}"')
    lines.append(")")
    # ------------------------------------------------------------
    # Output
    # ------------------------------------------------------------

    lines.append("set_target_properties(GameScripts PROPERTIES")
    lines.append(
        '    RUNTIME_OUTPUT_DIRECTORY_DEBUG '
        '"${PROJECT_ROOT}/build/Debug"'
    )
    lines.append(
        '    LIBRARY_OUTPUT_DIRECTORY_DEBUG '
        '"${PROJECT_ROOT}/build/Debug"'
    )
    lines.append(
        '    ARCHIVE_OUTPUT_DIRECTORY_DEBUG '
        '"${PROJECT_ROOT}/build/Debug"'
    )
    lines.append(")")
    lines.append("")

    # ------------------------------------------------------------
    # Includes
    # ------------------------------------------------------------

    lines.append("target_include_directories(GameScripts PRIVATE")
    lines.append('    "${ENGINE_ROOT}"')
    lines.append('    "${ENGINE_ROOT}/Base/include"')
    lines.append('    "${ENGINE_ROOT}/Managers/include"')
    lines.append('    "${ENGINE_ROOT}/Systems/include"')
    lines.append('    "${ENGINE_ROOT}/DataClasses/include"')
    lines.append('    "${ENGINE_ROOT}/Tools/include"')
    lines.append('    "${ENGINE_ROOT}/Collision"')
    lines.append('    "${ENGINE_ROOT}/Components"')
    lines.append('    "${ENGINE_ROOT}/UI"')
    lines.append('    "${ENGINE_ROOT}/Commands"')
    lines.append('    "${ENGINE_ROOT}/include"')
    lines.append('    "${ENGINE_ROOT}/imgui"')
    lines.append('    "${PROJECT_ROOT}/Assets/Scripts"')
    lines.append(")")
    lines.append("")

    # ------------------------------------------------------------
    # SFML
    # ------------------------------------------------------------

    lines.append("set(SFML_STATIC_LIBRARIES TRUE)")
    lines.append(
        'set(SFML_DIR "${ENGINE_ROOT}/lib/cmake/SFML")'
    )
    lines.append(
        "find_package(SFML 2.6 "
        "COMPONENTS graphics window system audio REQUIRED)"
    )

    lines.append(
        'target_link_directories(GameScripts PRIVATE '
        '"${ENGINE_ROOT}/build/Debug" '
        '"${ENGINE_ROOT}/lib")'
    )

    lines.append("target_link_libraries(GameScripts PRIVATE")
    lines.append("    ECSEngineCore")
    lines.append("    sfml-graphics-s-d")
    lines.append("    sfml-window-s-d")
    lines.append("    sfml-system-s-d")
    lines.append("    sfml-audio-s-d")
    lines.append("    opengl32")
    lines.append("    winmm")
    lines.append("    gdi32")
    lines.append("    freetype")
    lines.append("    flac")
    lines.append("    vorbis")
    lines.append("    vorbisenc")
    lines.append("    vorbisfile")
    lines.append("    ogg")
    lines.append("    openal32")
    lines.append(")")
    lines.append("")

    OUTPUT_CMAKE.write_text(
        "\n".join(lines) + "\n",
        encoding="utf-8"
    )

if __name__ == "__main__":
    GENERATED_DIR.mkdir(parents=True, exist_ok=True)
    scripts = find_serializable_scripts(SCRIPTS_DIR)
    generate_registry(scripts)
    generate_cmake(scripts)
    print(f"Generated project module files for {len(scripts)} scripts in {PROJECT_ROOT}")