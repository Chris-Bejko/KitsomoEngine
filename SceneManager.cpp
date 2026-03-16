#include "SceneManager.h"
#include "Engine.h"
#include "Logger.h"
#include <filesystem>

void SceneManager::Init()
{
    std::filesystem::create_directories("scenes/");
    LOG_INFO("SceneManager initialized");
}

void SceneManager::LoadScene(const std::string& sceneName, SceneLoadMode mode)
{
    std::string path = "scenes/" + sceneName + ".scene";
    
    if (!std::filesystem::exists(path))
    {
        LOG_WARNING("Scene not found: ", path.c_str());
        return;
    }

    if (mode == SceneLoadMode::Replace)
    {
        DestroyNonPersistentEntities();
        loadedScenes.clear();
    }

    Engine::get().Load(path);
    currentScene = sceneName;
    loadedScenes.push_back(sceneName);
    LOG_INFO("Loaded scene: ", sceneName.c_str());
}

void SceneManager::UnloadScene(const std::string& sceneName)
{
    // Remove from loaded list
    loadedScenes.erase(
        std::remove(loadedScenes.begin(), loadedScenes.end(), sceneName),
        loadedScenes.end()
    );
    LOG_INFO("Unloaded scene: ", sceneName.c_str());
}

void SceneManager::SaveCurrentScene()
{
    if (currentScene.empty())
    {
        LOG_WARNING("No current scene to save!");
        return;
    }
    SaveSceneAs(currentScene);
}

void SceneManager::SaveSceneAs(const std::string& sceneName)
{
    std::filesystem::create_directories("scenes/");
    std::string path = "scenes/" + sceneName + ".scene";
    Engine::get().Save(path);
    currentScene = sceneName;
    LOG_INFO("Saved scene: ", path.c_str());
}

void SceneManager::DontDestroyOnLoad(Entity* entity)
{
    persistentEntities.insert(entity);
    LOG_INFO("Entity marked as persistent: ", entity->GetName().c_str());
}

bool SceneManager::IsPersistent(Entity* entity)
{
    return persistentEntities.count(entity) > 0;
}

void SceneManager::DestroyNonPersistentEntities()
{
    auto manager = Engine::get().GetManager();
    for (auto& e : manager->GetEntities())
    {
        if (!IsPersistent(e.get()))
            e->Destroy();
    }
    // Remove destroyed persistent pointers (entities that were deleted externally)
    for (auto it = persistentEntities.begin(); it != persistentEntities.end();)
    {
        if (!(*it)->IsActive())
            it = persistentEntities.erase(it);
        else
            ++it;
    }
}

std::vector<std::string> SceneManager::GetAvailableScenes()
{
    std::vector<std::string> scenes;
    if (!std::filesystem::exists("scenes/")) return scenes;
    
    for (const auto& entry : std::filesystem::directory_iterator("scenes/"))
    {
        if (entry.path().extension() == ".scene")
            scenes.push_back(entry.path().stem().string());
    }
    return scenes;
}