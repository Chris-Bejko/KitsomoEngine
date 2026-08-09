#pragma once

#include <filesystem>
#include <string>
#include <atomic>
#include <vector>

#include "ComponentRegistry.h"
#include "FileWatcher.h"

class ProjectModuleLoader
{
public:
    ProjectModuleLoader();
    ~ProjectModuleLoader();

    bool LoadProjectModule(const std::filesystem::path &projectRoot);
    bool RebuildProjectModule(const std::filesystem::path &projectRoot);
    void UnloadProjectModule();

    void StartWatching(const std::filesystem::path &scriptsDirectory);
    void StopWatching();
    bool ConsumeReloadRequest();

    bool HasLoadedModule() const;
    const std::vector<std::string> &GetLoadedComponentNames() const;
    void RegisterLoadedComponent(const std::string &name);

private:
    bool BuildProjectModule(const std::filesystem::path &projectRoot);
    bool LoadCompiledModule(const std::filesystem::path &projectRoot);

    void *moduleHandle = nullptr;
    std::filesystem::path loadedDllPath;
    std::filesystem::path watchedScriptsDirectory;
    std::vector<std::string> loadedComponentNames;
    FileWatcher watcher;
    std::atomic<bool> reloadRequested = false;
    unsigned long long loadGeneration = 0;

};
