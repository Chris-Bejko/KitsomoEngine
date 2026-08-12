#include "ProjectExporter.h"

#include <fstream>
#include <sstream>
#include <cstdlib>

#include "Logger.h"
#include "ScriptCompiler.h"
#include "ProjectModuleLoader.h"
#include "Engine.h"

bool ProjectExporter::Export(
    const std::filesystem::path& projectRoot,
    const Settings& settings)
{
    try
    {
        const std::filesystem::path absoluteProjectRoot =
            std::filesystem::absolute(projectRoot);

        if (!std::filesystem::exists(absoluteProjectRoot))
        {
            LOG_ERROR(
                "Project root does not exist: ",
                absoluteProjectRoot.string().c_str());

            return false;
        }

        if (settings.outputDirectory.empty())
        {
            LOG_ERROR("Export output directory is empty");
            return false;
        }

        const std::filesystem::path exportRoot =
            std::filesystem::absolute(
                settings.outputDirectory);

        LOG_INFO(
            "============================================");

        LOG_INFO(
            "Exporting project: ",
            settings.gameName.c_str());

        LOG_INFO(
            "Output: ",
            exportRoot.string().c_str());

        LOG_INFO(
            "============================================");

        // --------------------------------------------------------
        // Clean old export
        // --------------------------------------------------------

        std::error_code ec;

        if (std::filesystem::exists(exportRoot))
        {
            std::filesystem::remove_all(
                exportRoot,
                ec);

            if (ec)
            {
                LOG_ERROR(
                    "Failed to clean export directory: ",
                    exportRoot.string().c_str());

                return false;
            }
        }

        std::filesystem::create_directories(
            exportRoot);

        // --------------------------------------------------------
        // IMPORTANT:
        //
        // Make sure GameScripts.dll exists before exporting.
        //
        // We use the existing project build.
        // --------------------------------------------------------

        const std::filesystem::path gameScripts =
            absoluteProjectRoot /
            "build" /
            "Debug" /
            "GameScripts.dll";

        if (!std::filesystem::exists(gameScripts))
        {
            LOG_ERROR(
                "GameScripts.dll was not found. "
                "Build the project before exporting: ",
                gameScripts.string().c_str());

            return false;
        }

        // --------------------------------------------------------
        // Generate runtime project
        // --------------------------------------------------------

        if (!GenerateRuntimeFiles(
                absoluteProjectRoot,
                exportRoot,
                settings))
        {
            return false;
        }

        // --------------------------------------------------------
        // Copy game DLL
        // --------------------------------------------------------

        if (!CopyGameScripts(
                absoluteProjectRoot,
                exportRoot))
        {
            return false;
        }

        // --------------------------------------------------------
        // Copy assets
        // --------------------------------------------------------

        if (settings.copyAssets)
        {
            //project
            if (!CopyProjectAssets(
                    absoluteProjectRoot,
                    exportRoot))
            {
                return false;
            }
            const std::filesystem::path engineRoot = std::filesystem::current_path();

            //engine
            if(!CopyProjectAssets(
                    engineRoot,
                    exportRoot))
            {
                return false;
            }
        }

        // --------------------------------------------------------
        // Copy engine/runtime dependencies
        // --------------------------------------------------------

        if (!CopyRuntimeDependencies(
                absoluteProjectRoot,
                exportRoot,
                settings))
        {
            return false;
        }

        // --------------------------------------------------------
        // Build executable
        // --------------------------------------------------------

        if (!BuildExport(
                exportRoot,
                settings))
        {
            return false;
        }

        LOG_INFO(
            "============================================");

        LOG_INFO(
            "EXPORT SUCCESSFUL");

        LOG_INFO(
            "Executable: ",
            (
                exportRoot /
                (settings.executableName + ".exe")
            ).string().c_str());

        LOG_INFO(
            "============================================");

        return true;
    }
    catch (const std::exception& e)
    {
        LOG_ERROR(
            "Export exception: ",
            e.what());

        return false;
    }
}

bool ProjectExporter::GenerateRuntimeFiles(
    const std::filesystem::path& projectRoot,
    const std::filesystem::path& exportRoot,
    const Settings& settings)
{
    const std::filesystem::path generated =
        exportRoot / "Generated";

    std::error_code ec;

    std::filesystem::create_directories(
        generated,
        ec);

    if (ec)
    {
        LOG_ERROR(
            "Failed to create Generated directory");

        return false;
    }

    if (!WriteRuntimeMain(
            exportRoot,
            settings))
    {
        return false;
    }

    if (!WriteRuntimeCMake(
            projectRoot,
            exportRoot,
            settings))
    {
        return false;
    }

    return true;
}

bool ProjectExporter::WriteRuntimeMain(
    const std::filesystem::path& exportRoot,
    const Settings& settings)
{
    const std::filesystem::path mainFile =
        exportRoot / "Generated" / "RuntimeMain.cpp";

    std::ofstream file(mainFile);

    if (!file)
    {
        LOG_ERROR(
            "Failed to create runtime main.cpp");

        return false;
    }

    file << R"(#include "Engine.h"
#include "Logger.h"

int main()
{
    Engine& engine = Engine::get();

    engine.InitRuntime(
        ".",
        ")" << settings.startupScene << R"("
    );

    while (engine.IsRunning())
    {
        engine.Events();
        engine.Update();
        engine.RenderRuntime();
    }

    return 0;
}
)";

    return true;
}


bool ProjectExporter::WriteRuntimeCMake(
    const std::filesystem::path& projectRoot,
    const std::filesystem::path& exportRoot,
    const Settings& settings)
{
    const std::filesystem::path cmakeFile =
        exportRoot / "CMakeLists.txt";

    std::ofstream file(cmakeFile);

    if (!file)
    {
        LOG_ERROR(
            "Failed to create runtime CMakeLists.txt");

        return false;
    }

    const std::string engineRoot =
        std::filesystem::absolute(
            projectRoot.parent_path().parent_path()
        ).string();

    file << "cmake_minimum_required(VERSION 3.20)\n";
    file << "project("
         << settings.executableName
         << ")\n\n";

    file << "set(CMAKE_CXX_STANDARD 17)\n";
    file << "set(CMAKE_CXX_STANDARD_REQUIRED ON)\n";
    file << "set(CMAKE_CXX_EXTENSIONS OFF)\n\n";

    file << "set(ENGINE_ROOT \""
         << EscapeCMake(engineRoot)
         << "\")\n";

    file << "set(EXPORT_ROOT \""
         << EscapeCMake(exportRoot.string())
         << "\")\n\n";

    file << "add_compile_definitions(\n";
    file << "    SFML_STATIC\n";
    file << "    NOMINMAX\n";
    file << "    WIN32_LEAN_AND_MEAN\n";
    file << ")\n\n";

    // ------------------------------------------------------------
    // Runtime executable
    // ------------------------------------------------------------

    file << "add_executable("
         << settings.executableName
         << "\n";

    file << "    \"${EXPORT_ROOT}/Generated/RuntimeMain.cpp\"\n";

    file << ")\n\n";

    // ------------------------------------------------------------
    // Includes
    // ------------------------------------------------------------

    file << "target_include_directories("
         << settings.executableName
         << " PRIVATE\n";

    file << "    \"${ENGINE_ROOT}\"\n";
    file << "    \"${ENGINE_ROOT}/Base/include\"\n";
    file << "    \"${ENGINE_ROOT}/Managers/include\"\n";
    file << "    \"${ENGINE_ROOT}/Systems/include\"\n";
    file << "    \"${ENGINE_ROOT}/DataClasses/include\"\n";
    file << "    \"${ENGINE_ROOT}/Tools/include\"\n";
    file << "    \"${ENGINE_ROOT}/Collision\"\n";
    file << "    \"${ENGINE_ROOT}/Components\"\n";
    file << "    \"${ENGINE_ROOT}/UI\"\n";
    file << "    \"${ENGINE_ROOT}/Commands\"\n";
    file << "    \"${ENGINE_ROOT}/include\"\n";
    file << "    \"${ENGINE_ROOT}/imgui\"\n";

    file << ")\n\n";

    // ------------------------------------------------------------
    // Engine library
    // ------------------------------------------------------------

    file << "target_link_directories("
         << settings.executableName
         << " PRIVATE\n";

    file << "    \"${ENGINE_ROOT}/build/Debug\"\n";
    file << "    \"${ENGINE_ROOT}/lib\"\n";

    file << ")\n\n";

    // ------------------------------------------------------------
    // SFML
    // ------------------------------------------------------------

    file << "set(SFML_STATIC_LIBRARIES TRUE)\n";

    file << "set(SFML_DIR "
            "\"${ENGINE_ROOT}/lib/cmake/SFML\")\n";

    file << "find_package(SFML 2.6 "
            "COMPONENTS graphics window system audio REQUIRED)\n\n";

    // ------------------------------------------------------------
    // Link
    // ------------------------------------------------------------

    file << "target_link_libraries("
         << settings.executableName
         << " PRIVATE\n";

    file << "    ECSEngineCore\n";

    file << "    sfml-graphics-s-d\n";
    file << "    sfml-window-s-d\n";
    file << "    sfml-system-s-d\n";
    file << "    sfml-audio-s-d\n";

    file << "    opengl32\n";
    file << "    winmm\n";
    file << "    gdi32\n";
    file << "    freetype\n";
    file << "    flac\n";
    file << "    vorbis\n";
    file << "    vorbisenc\n";
    file << "    vorbisfile\n";
    file << "    ogg\n";
    file << "    openal32\n";

    file << ")\n\n";

    // ------------------------------------------------------------
    // Put executable directly in export root
    // ------------------------------------------------------------

    file << "set_target_properties("
         << settings.executableName
         << " PROPERTIES\n";

    file << "    RUNTIME_OUTPUT_DIRECTORY_DEBUG "
            "\"${EXPORT_ROOT}\"\n";

    file << "    RUNTIME_OUTPUT_DIRECTORY_RELEASE "
            "\"${EXPORT_ROOT}\"\n";

    file << ")\n";

    return true;
}


bool ProjectExporter::CopyGameScripts(
    const std::filesystem::path& projectRoot,
    const std::filesystem::path& exportRoot)
{
    const std::filesystem::path source =
        projectRoot /
        "build" /
        "Debug" /
        "GameScripts.dll";

    const std::filesystem::path destination =
        exportRoot /
        "build" /
        "Debug" /
        "GameScripts.dll";

    std::error_code ec;

    std::filesystem::create_directories(
        destination.parent_path(),
        ec);

    if (ec)
    {
        LOG_ERROR(
            "Failed to create runtime DLL directory");

        return false;
    }

    return CopyFileIfExists(
        source,
        destination);
}


bool ProjectExporter::CopyProjectAssets(
    const std::filesystem::path& sourceRoot,
    const std::filesystem::path& exportRoot)
{
    const std::filesystem::path source =
        sourceRoot / "Assets";

    const std::filesystem::path destination =
        exportRoot / "Assets";

    if (!std::filesystem::exists(source))
    {
        LOG_ERROR(
            "Project Assets directory does not exist: ",
            source.string().c_str());

        return false;
    }

    LOG_INFO(
        "Copying project assets...");

    std::error_code ec;

    std::filesystem::copy(
        source,
        destination,
        std::filesystem::copy_options::recursive |
        std::filesystem::copy_options::overwrite_existing,
        ec);

    if (ec)
    {
        LOG_ERROR(
            "Failed to copy project assets: ",
            ec.message().c_str());

        return false;
    }

    return true;
}


bool ProjectExporter::CopyRuntimeDependencies(
    const std::filesystem::path& projectRoot,
    const std::filesystem::path& exportRoot,
    const Settings& settings)
{
    const std::filesystem::path engineRoot =
        std::filesystem::absolute(
            projectRoot.parent_path().parent_path());

    // ------------------------------------------------------------
    // ECSEngineCore.dll
    // ------------------------------------------------------------

    const std::filesystem::path engineDll =
        engineRoot /
        "build" /
        "Debug" /
        "ECSEngineCore.dll";

    if (!CopyFileIfExists(
            engineDll,
            exportRoot / "ECSEngineCore.dll"))
    {
        LOG_ERROR(
            "Failed to copy ECSEngineCore.dll");

        return false;
    }

    // ------------------------------------------------------------
    // OpenAL
    // ------------------------------------------------------------

    if (settings.copyOpenAL)
    {
        const std::filesystem::path openAL =
            engineRoot /
            "bin" /
            "OpenAL32.dll";

        if (std::filesystem::exists(openAL))
        {
            if (!CopyFileIfExists(
                    openAL,
                    exportRoot / "OpenAL32.dll"))
            {
                return false;
            }
        }
        else
        {
            LOG_WARNING(
                "OpenAL32.dll not found: ",
                openAL.string().c_str());
        }
    }

    return true;
}


bool ProjectExporter::CopyFileIfExists(
    const std::filesystem::path& source,
    const std::filesystem::path& destination)
{
    if (!std::filesystem::exists(source))
    {
        LOG_ERROR(
            "Required file does not exist: ",
            source.string().c_str());

        return false;
    }

    std::error_code ec;

    std::filesystem::create_directories(
        destination.parent_path(),
        ec);

    if (ec)
    {
        LOG_ERROR(
            "Failed to create destination directory: ",
            destination.parent_path().string().c_str());

        return false;
    }

    std::filesystem::copy_file(
        source,
        destination,
        std::filesystem::copy_options::overwrite_existing,
        ec);

    if (ec)
    {
        LOG_ERROR(
            "Failed to copy ",
            source.string().c_str(),
            " -> ",
            destination.string().c_str(),
            ": ",
            ec.message().c_str());

        return false;
    }

    LOG_INFO(
        "Copied: ",
        source.filename().string().c_str());

    return true;
}


bool ProjectExporter::BuildExport(
    const std::filesystem::path& exportRoot,
    const Settings& settings)
{
    const std::filesystem::path buildDirectory =
        exportRoot / "Build";

    std::error_code ec;

    std::filesystem::create_directories(
        buildDirectory,
        ec);

    if (ec)
    {
        LOG_ERROR(
            "Failed to create export build directory");

        return false;
    }

    // ------------------------------------------------------------
    // Configure
    // ------------------------------------------------------------

    std::string configureCommand =
        "cmake "
        "-S \"" +
        exportRoot.string() +
        "\" "
        "-B \"" +
        buildDirectory.string() +
        "\"";

    LOG_INFO(
        "Configuring standalone runtime...");

    if (!RunCommand(
            configureCommand,
            exportRoot))
    {
        return false;
    }

    // ------------------------------------------------------------
    // Build
    // ------------------------------------------------------------

    std::string buildCommand =
        "cmake --build \"" +
        buildDirectory.string() +
        "\" "
        "--config Debug";

    LOG_INFO(
        "Building standalone executable...");

    if (!RunCommand(
            buildCommand,
            exportRoot))
    {
        return false;
    }

    // ------------------------------------------------------------
    // Verify executable
    // ------------------------------------------------------------

    const std::filesystem::path executable =
        exportRoot /
        (settings.executableName + ".exe");

    if (!std::filesystem::exists(executable))
    {
        LOG_ERROR(
            "CMake completed but executable was not found: ",
            executable.string().c_str());

        return false;
    }

    return true;
}


std::string ProjectExporter::EscapeCMake(
    const std::string& value)
{
    std::string result;

    for (char c : value)
    {
        if (c == '\\')
        {
            result += '/';
        }
        else if (c == '"')
        {
            result += "\\\"";
        }
        else
        {
            result += c;
        }
    }

    return result;
}

bool ProjectExporter::RunCommand(
    const std::string& command,
    const std::filesystem::path& workingDirectory)
{
    LOG_INFO(
        "COMMAND: ",
        command.c_str());

    const std::string fullCommand =
        "cd /d \"" +
        workingDirectory.string() +
        "\" && " +
        command;

    const int result =
        std::system(fullCommand.c_str());

    if (result != 0)
    {
        LOG_ERROR(
            "Command failed with exit code: ",
            std::to_string(result).c_str());

        return false;
    }

    return true;
}