#pragma once
#include <string>
#include <vector>
#include <set>
#include <filesystem>
#include "Entity.h"

enum class SceneLoadMode
{
    Replace,  // destroy current scene, load new
    Additive  // keep current scene, add new on top
};

class SceneManager
{
public:
    static SceneManager& get()
    {
        static SceneManager instance;
        return instance;
    }

    void LoadScene(const std::string& sceneName, SceneLoadMode mode = SceneLoadMode::Replace);
    void UnloadScene(const std::string& sceneName);
    void SaveCurrentScene();
    void SaveSceneAs(const std::string& sceneName);
    bool CreateNewProject(const std::string& projectName, const std::string& baseDirectory = "Projects");
    bool OpenProject(const std::string& path);
    void ClearProjectRoot();
    void SetProjectRoot(const std::string& path);
    std::string GetProjectRoot() const;
    std::filesystem::path GetProjectRootPath() const;
    std::filesystem::path GetSceneDirectory() const;
    std::filesystem::path GetPrefabDirectory() const;
    std::filesystem::path GetScriptsDirectory() const;
    std::filesystem::path ResolveProjectPath(const std::string& relativePath) const;
    std::vector<std::filesystem::path> GetAvailableProjects(const std::string& baseDirectory = "Projects") const;
    bool HasProjectRoot() const;

    // DontDestroyOnLoad
    void DontDestroyOnLoad(Entity* entity);
    bool IsPersistent(Entity* entity);

    // Getters
    std::string GetCurrentScene() { return currentScene; }
    std::vector<std::string> GetAvailableScenes();
    std::vector<std::string> GetLoadedScenes() { return loadedScenes; }

    void Init();

private:
    SceneManager() = default;
    std::string currentScene = "";
    std::vector<std::string> loadedScenes;
    std::set<Entity*> persistentEntities;
    std::filesystem::path projectRoot;

    void DestroyNonPersistentEntities();
};