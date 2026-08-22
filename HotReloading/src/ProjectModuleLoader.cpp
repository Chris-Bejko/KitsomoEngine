#include "ProjectModuleLoader.h"

#include <Windows.h>

#include <filesystem>
#include <string>

#include "ComponentRegistry.h"
#include "ProjectModuleAPI.h"
#include "ScriptCompiler.h"
#include "Logger.h"
#include "Engine.h"
#include <Windows.h>


namespace
{
    ProjectModuleLoader *g_activeModuleLoader = nullptr;

    void RegisterProjectComponentFromModule(
        const char *name,
        bool allowsMultiple,
        ComponentAddDefaultFn addDefault,
        ComponentApplySerializedFn applySerialized)
    {
        LOG_INFO(
            "DLL requested registration: ",
            name ? name : "<null>");

        if (g_activeModuleLoader == nullptr ||
            name == nullptr)
        {
            LOG_ERROR("Project component registration callback rejected");
            return;
        }

        const bool registered =
            ComponentRegistry::get().RegisterExternal(
                name,
                allowsMultiple,
                addDefault,
                applySerialized,
                true);

        LOG_INFO(
            "RegisterExternal result for ",
            name,
            ": ",
            registered ? "SUCCESS" : "FAILED");

        if (registered)
        {
            g_activeModuleLoader->RegisterLoadedComponent(name);
        }
    }
}

// ============================================================
// Construction / Destruction
// ============================================================

ProjectModuleLoader::ProjectModuleLoader() = default;

ProjectModuleLoader::~ProjectModuleLoader()
{
    StopWatching();

    // The engine should normally have destroyed all entities
    // before this point.
    UnloadProjectModule();
}

// ============================================================
// Editor loading
//
// Build the project DLL, then load a LIVE COPY of it.
//
// This is used by the editor because the original
// GameScripts.dll must remain available for rebuilding.
// ============================================================

bool ProjectModuleLoader::LoadProjectModule(
    const std::filesystem::path &projectRoot)
{
    if (!BuildProjectModule(projectRoot))
    {
        return false;
    }

    return LoadCompiledModule(projectRoot);
}

// ============================================================
// Runtime loading
//
// DO NOT BUILD.
// DO NOT RUN PYTHON.
// DO NOT RUN CMAKE.
//
// Simply load the already compiled GameScripts.dll.
// ============================================================
bool ProjectModuleLoader::LoadCompiledProjectModule(
    const std::filesystem::path &projectRoot)
{
    LOG_INFO("DLL 1: LoadCompiledProjectModule BEGIN");

    const std::filesystem::path sourceDll =
        projectRoot /
        "build" /
        "Debug" /
        "GameScripts.dll";

    LOG_INFO(
        "DLL 2: DLL path = ",
        sourceDll.string().c_str());

    if (!std::filesystem::exists(sourceDll))
    {
        LOG_ERROR(
            "DLL 3: GameScripts.dll does not exist");

        return false;
    }

    LOG_INFO("DLL 4: Calling LoadLibrary");

    HMODULE loadedModule =
        LoadLibraryA(
            sourceDll.string().c_str());

    if (loadedModule == nullptr)
    {
        const DWORD error = GetLastError();

        LOG_ERROR(
            "DLL 5: LoadLibrary FAILED. Error = ",
            std::to_string(error).c_str());

        return false;
    }

    LOG_INFO("DLL 6: LoadLibrary SUCCESS");

    auto registerFn =
        reinterpret_cast<RegisterProjectComponentsExport>(
            GetProcAddress(
                loadedModule,
                "RegisterProjectComponents"));

    if (registerFn == nullptr)
    {
        const DWORD error = GetLastError();

        LOG_ERROR(
            "DLL 7: RegisterProjectComponents NOT FOUND. Error = ",
            std::to_string(error).c_str());

        FreeLibrary(loadedModule);

        return false;
    }

    LOG_INFO("DLL 8: RegisterProjectComponents found");

    moduleHandle = loadedModule;
    loadedDllPath = sourceDll;

    loadedComponentNames.clear();

    LOG_INFO("DLL 9: Calling RegisterProjectComponents");

    g_activeModuleLoader = this;

    registerFn(
        &RegisterProjectComponentFromModule);

    g_activeModuleLoader = nullptr;

    LOG_INFO("DLL 10: RegisterProjectComponents returned");

    return true;
}
// ============================================================
// Editor hot reload
//
// Build new DLL
// Destroy objects using old DLL
// Remove old registry callbacks
// Unload old DLL
// Load new DLL
// ============================================================

bool ProjectModuleLoader::RebuildProjectModule(
    const std::filesystem::path &projectRoot)
{
    // --------------------------------------------------------
    // Build new project DLL
    // --------------------------------------------------------

    if (!BuildProjectModule(projectRoot))
    {
        return false;
    }

    // --------------------------------------------------------
    // Destroy objects created by the old DLL
    //
    // This MUST happen before FreeLibrary().
    // --------------------------------------------------------

    Engine::get().PrepareForProjectModuleUnload();

    // --------------------------------------------------------
    // Remove descriptors/function pointers belonging to
    // the old project DLL.
    // --------------------------------------------------------

    ComponentRegistry::get()
        .UnregisterProjectComponents();

    // --------------------------------------------------------
    // Unload old live DLL.
    // --------------------------------------------------------

    UnloadProjectModule();

    // --------------------------------------------------------
    // Load the newly compiled DLL as a fresh live copy.
    // --------------------------------------------------------

    return LoadCompiledModule(projectRoot);
}

// ============================================================
// LoadCompiledModule
//
// EDITOR ONLY.
//
// Takes:
//
//     build/Debug/GameScripts.dll
//
// and copies it to:
//
//     Generated/LoadedModules/GameScripts_live_N.dll
//
// Then loads that copy.
//
// This prevents the editor from locking the actual build DLL,
// allowing CMake/MSBuild to rebuild it during hot reload.
// ============================================================

bool ProjectModuleLoader::LoadCompiledModule(
    const std::filesystem::path &projectRoot)
{
    const std::filesystem::path sourceDll =
        projectRoot /
        "build" /
        "Debug" /
        "GameScripts.dll";

    // --------------------------------------------------------
    // Verify source DLL
    // --------------------------------------------------------

    if (!std::filesystem::exists(sourceDll))
    {
        LOG_ERROR(
            "Compiled project module not found: ",
            sourceDll.string().c_str());

        return false;
    }

    // --------------------------------------------------------
    // Create live module directory
    // --------------------------------------------------------

    const std::filesystem::path liveDirectory =
        projectRoot /
        "Generated" /
        "LoadedModules";

    std::error_code error;

    std::filesystem::create_directories(
        liveDirectory,
        error);

    if (error)
    {
        LOG_ERROR(
            "Failed to create live module directory: ",
            liveDirectory.string().c_str());

        return false;
    }

    // --------------------------------------------------------
    // Generate unique DLL name
    // --------------------------------------------------------

    ++loadGeneration;

    const std::filesystem::path liveDll =
        liveDirectory /
        ("GameScripts_live_" +
         std::to_string(loadGeneration) +
         ".dll");

    // --------------------------------------------------------
    // Copy compiled DLL
    // --------------------------------------------------------

    std::filesystem::copy_file(
        sourceDll,
        liveDll,
        std::filesystem::copy_options::overwrite_existing,
        error);

    if (error)
    {
        LOG_ERROR(
            "Failed to copy project DLL to live module: ",
            error.message().c_str());

        return false;
    }

    LOG_INFO(
        "Loading live project module: ",
        liveDll.string().c_str());

    // --------------------------------------------------------
    // Load DLL
    // --------------------------------------------------------

    HMODULE loadedModule =
        LoadLibraryA(
            liveDll.string().c_str());

    if (loadedModule == nullptr)
    {
        LOG_ERROR(
            "Failed to load project module: ",
            liveDll.string().c_str());

        std::filesystem::remove(
            liveDll,
            error);

        return false;
    }

    // --------------------------------------------------------
    // Find registration function
    // --------------------------------------------------------

    auto registerFn =
        reinterpret_cast<RegisterProjectComponentsExport>(
            GetProcAddress(
                loadedModule,
                "RegisterProjectComponents"));

    if (registerFn == nullptr)
    {
        LOG_ERROR(
            "RegisterProjectComponents export not found in project module");

        FreeLibrary(loadedModule);

        std::filesystem::remove(
            liveDll,
            error);

        return false;
    }

    // --------------------------------------------------------
    // Store module information
    // --------------------------------------------------------

    moduleHandle = loadedModule;

    // This IS a GameScripts_live_N.dll file.
    // UnloadProjectModule() is therefore allowed to delete it.
    loadedDllPath = liveDll;

    loadedComponentNames.clear();

    // --------------------------------------------------------
    // Register project components
    // --------------------------------------------------------

    g_activeModuleLoader = this;

    registerFn(
        &RegisterProjectComponentFromModule);

    g_activeModuleLoader = nullptr;

    LOG_INFO(
        "Project module loaded successfully.");

    return true;
}

// ============================================================
// UnloadProjectModule
//
// Unloads whichever DLL is currently loaded.
//
// IMPORTANT:
//
// Only delete files that were created by the editor's
// live-copy system.
//
// NEVER delete:
//
//     build/Debug/GameScripts.dll
//
// because that is the actual project/runtime DLL.
// ============================================================

void ProjectModuleLoader::UnloadProjectModule()
{
    // --------------------------------------------------------
    // Unload DLL
    // --------------------------------------------------------

    if (moduleHandle != nullptr)
    {
        FreeLibrary(
            reinterpret_cast<HMODULE>(
                moduleHandle));

        moduleHandle = nullptr;
    }

    // --------------------------------------------------------
    // Delete editor-created live DLL only
    // --------------------------------------------------------

    if (!loadedDllPath.empty())
    {
        const std::string filename =
            loadedDllPath.filename().string();

        const bool isLiveModule =
            filename.rfind(
                "GameScripts_live_",
                0) == 0;

        if (isLiveModule &&
            std::filesystem::exists(
                loadedDllPath))
        {
            std::error_code error;

            std::filesystem::remove(
                loadedDllPath,
                error);

            if (error)
            {
                LOG_WARNING(
                    "Failed to remove old live project module: ",
                    loadedDllPath.string().c_str(),
                    " : ",
                    error.message().c_str());
            }
        }
    }

    loadedDllPath.clear();
    loadedComponentNames.clear();
}

// ============================================================
// Hot reload watcher
// ============================================================

void ProjectModuleLoader::StartWatching(
    const std::filesystem::path &scriptsDirectory)
{
    StopWatching();

    watchedScriptsDirectory =
        scriptsDirectory;

    if (watchedScriptsDirectory.empty() ||
        !std::filesystem::exists(
            watchedScriptsDirectory))
    {
        return;
    }

    watcher.Watch(
        watchedScriptsDirectory.string(),
        [this](std::string)
        {
            reloadRequested = true;
        });
}

// ============================================================

void ProjectModuleLoader::StopWatching()
{
    watcher.Stop();
}

// ============================================================

bool ProjectModuleLoader::ConsumeReloadRequest()
{
    if (!reloadRequested)
    {
        return false;
    }

    reloadRequested = false;

    return true;
}

// ============================================================
// State
// ============================================================

bool ProjectModuleLoader::HasLoadedModule() const
{
    return moduleHandle != nullptr;
}

// ============================================================

const std::vector<std::string> &
ProjectModuleLoader::GetLoadedComponentNames() const
{
    return loadedComponentNames;
}

// ============================================================
// Build
// ============================================================

bool ProjectModuleLoader::BuildProjectModule(
    const std::filesystem::path &projectRoot)
{
    return ScriptCompiler::GenerateAndCompile(
        projectRoot.string());
}

// ============================================================
// Component tracking
// ============================================================

void ProjectModuleLoader::RegisterLoadedComponent(
    const std::string &name)
{
    loadedComponentNames.push_back(name);
}