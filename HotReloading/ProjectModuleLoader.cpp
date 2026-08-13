#include "ProjectModuleLoader.h"

#include <Windows.h>

#include "ComponentRegistry.h"
#include "HotReloading/ProjectModuleAPI.h"
#include "HotReloading/ScriptCompiler.h"
#include "Logger.h"
#include "Engine.h"

namespace
{
    ProjectModuleLoader* g_activeModuleLoader = nullptr;

    void RegisterProjectComponentFromModule(
        const char* name,
        bool allowsMultiple,
        ComponentAddDefaultFn addDefault,
        ComponentApplySerializedFn applySerialized)
    {
        if (g_activeModuleLoader == nullptr || name == nullptr)
        {
            return;
        }

        if (ComponentRegistry::get().RegisterExternal(
                name,
                allowsMultiple,
                addDefault,
                applySerialized,
                true))
        {
            g_activeModuleLoader->RegisterLoadedComponent(name);
        }
    }
}

ProjectModuleLoader::ProjectModuleLoader() = default;

ProjectModuleLoader::~ProjectModuleLoader()
{
    StopWatching();

    // Project components must be destroyed by the engine
    // before the DLL containing their implementations is unloaded.
    UnloadProjectModule();
}

bool ProjectModuleLoader::LoadProjectModule(
    const std::filesystem::path& projectRoot)
{
    if (!BuildProjectModule(projectRoot))
    {
        return false;
    }

    return LoadCompiledModule(projectRoot);
}

bool ProjectModuleLoader::RebuildProjectModule(
    const std::filesystem::path& projectRoot)
{
    if (!BuildProjectModule(projectRoot))
        return false;

    // 1. DESTROY OBJECTS CREATED BY THE DLL
    Engine::get().PrepareForProjectModuleUnload();

    // 2. Remove descriptors/function pointers into DLL
    ComponentRegistry::get().UnregisterProjectComponents();

    // 3. NOW unload DLL
    UnloadProjectModule();

    // 4. Load new DLL
    return LoadCompiledModule(projectRoot);
}

void ProjectModuleLoader::UnloadProjectModule()
{
    /*
        The registry should normally already have been cleared by
        RebuildProjectModule() before reaching here.

        This function therefore only unloads the actual module.
    */
    if (moduleHandle != nullptr)
    {
        FreeLibrary(static_cast<HMODULE>(moduleHandle));
        moduleHandle = nullptr;
    }

    if (!loadedDllPath.empty() &&
        std::filesystem::exists(loadedDllPath))
    {
        std::error_code error;
        std::filesystem::remove(loadedDllPath, error);
    }

    loadedDllPath.clear();
    loadedComponentNames.clear();
}

void ProjectModuleLoader::StartWatching(
    const std::filesystem::path& scriptsDirectory)
{
    StopWatching();

    watchedScriptsDirectory = scriptsDirectory;

    if (watchedScriptsDirectory.empty() ||
        !std::filesystem::exists(watchedScriptsDirectory))
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

void ProjectModuleLoader::StopWatching()
{
    watcher.Stop();
}

bool ProjectModuleLoader::ConsumeReloadRequest()
{
    if (!reloadRequested)
    {
        return false;
    }

    reloadRequested = false;
    return true;
}

bool ProjectModuleLoader::HasLoadedModule() const
{
    return moduleHandle != nullptr;
}

const std::vector<std::string>&
ProjectModuleLoader::GetLoadedComponentNames() const
{
    return loadedComponentNames;
}

bool ProjectModuleLoader::BuildProjectModule(
    const std::filesystem::path& projectRoot)
{
    return ScriptCompiler::GenerateAndCompile(projectRoot.string());
}

bool ProjectModuleLoader::LoadCompiledModule(
    const std::filesystem::path& projectRoot)
{
    std::filesystem::path sourceDll =
        projectRoot /
        "build" /
        "Debug" /
        "GameScripts.dll";

    if (!std::filesystem::exists(sourceDll))
    {
        const std::filesystem::path fallbackDll =
            projectRoot /
            "build" /
            "Debug" /
            "Debug" /
            "GameScripts.dll";

        if (std::filesystem::exists(fallbackDll))
        {
            sourceDll = fallbackDll;
        }
    }

    if (!std::filesystem::exists(sourceDll))
    {
        LOG_ERROR(
            "Compiled script module not found: ",
            sourceDll.string().c_str());

        return false;
    }

    const std::filesystem::path liveDir =
        projectRoot /
        "Generated" /
        "LoadedModules";

    std::filesystem::create_directories(liveDir);

    loadedDllPath =
        liveDir /
        ("GameScripts_live_" +
         std::to_string(++loadGeneration) +
         ".dll");

    std::filesystem::copy_file(
        sourceDll,
        loadedDllPath,
        std::filesystem::copy_options::overwrite_existing);

    HMODULE loadedModule =
        LoadLibraryA(loadedDllPath.string().c_str());

    if (loadedModule == nullptr)
    {
        LOG_ERROR(
            "Failed to load script module: ",
            loadedDllPath.string().c_str());

        return false;
    }

    auto registerFn =
        reinterpret_cast<RegisterProjectComponentsExport>(
            GetProcAddress(
                loadedModule,
                "RegisterProjectComponents"));

    if (registerFn == nullptr)
    {
        LOG_ERROR(
            "RegisterProjectComponents export not found in script module");

        FreeLibrary(loadedModule);

        return false;
    }

    moduleHandle = loadedModule;
    loadedComponentNames.clear();

    g_activeModuleLoader = this;

    registerFn(&RegisterProjectComponentFromModule);

    g_activeModuleLoader = nullptr;

    return true;
}

void ProjectModuleLoader::RegisterLoadedComponent(
    const std::string& name)
{
    loadedComponentNames.push_back(name);
}