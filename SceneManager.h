#pragma once
#include <string>
#include <vector>
#include <set>
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

    void DestroyNonPersistentEntities();
};