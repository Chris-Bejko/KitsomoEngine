#include "SceneManager.h"
#include "Engine.h"
#include "Logger.h"
#include <filesystem>
#include <algorithm>

void SceneManager::Init()
{
    if (!HasProjectRoot())
    {
        std::filesystem::path defaultRoot = std::filesystem::current_path() / "Projects" / "DefaultProject";
        SetProjectRoot(defaultRoot.string());
    }

    std::filesystem::create_directories(GetSceneDirectory());
    std::filesystem::create_directories(GetPrefabDirectory());
    LOG_INFO("SceneManager initialized");
}

void SceneManager::LoadScene(const std::string& sceneName, SceneLoadMode mode)
{
    std::filesystem::path path = GetSceneDirectory() / (sceneName + ".scene");
    
    if (!std::filesystem::exists(path))
    {
        LOG_WARNING("Scene not found: ", path.string().c_str());
        return;
    }

    if (mode == SceneLoadMode::Replace)
    {
        DestroyNonPersistentEntities();
        loadedScenes.clear();
    }

    Engine::get().Load(path.string());
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
    std::filesystem::create_directories(GetSceneDirectory());
    std::filesystem::path path = GetSceneDirectory() / (sceneName + ".scene");
    Engine::get().Save(path.string());
    currentScene = sceneName;
    LOG_INFO("Saved scene: ", path.string().c_str());
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

bool SceneManager::CreateNewProject(const std::string& projectName, const std::string& baseDirectory)
{
    if (projectName.empty())
        return false;

    std::filesystem::path base(baseDirectory.empty() ? "Projects" : baseDirectory);
    std::filesystem::path root = std::filesystem::absolute(base / projectName);
    std::filesystem::create_directories(root / "Assets" / "Scenes");
    std::filesystem::create_directories(root / "Assets" / "Prefabs");
    std::filesystem::create_directories(root / "Assets" / "Scripts");

    SetProjectRoot(root.string());
    currentScene.clear();
    loadedScenes.clear();
    return true;
}

void SceneManager::SetProjectRoot(const std::string& path)
{
    projectRoot = std::filesystem::absolute(path);
    std::filesystem::create_directories(projectRoot);
}

std::string SceneManager::GetProjectRoot() const
{
    return projectRoot.empty() ? std::string() : projectRoot.string();
}

std::filesystem::path SceneManager::GetProjectRootPath() const
{
    return projectRoot.empty() ? std::filesystem::current_path() : projectRoot;
}

std::filesystem::path SceneManager::GetSceneDirectory() const
{
    return ResolveProjectPath("Assets/Scenes");
}

std::filesystem::path SceneManager::GetPrefabDirectory() const
{
    return ResolveProjectPath("Assets/Prefabs");
}

std::filesystem::path SceneManager::ResolveProjectPath(const std::string& relativePath) const
{
    if (relativePath.empty())
        return GetProjectRootPath();

    std::filesystem::path path(relativePath);
    if (path.is_absolute())
        return path;

    return GetProjectRootPath() / path;
}

bool SceneManager::HasProjectRoot() const
{
    return !projectRoot.empty();
}

std::vector<std::string> SceneManager::GetAvailableScenes()
{
    std::vector<std::string> scenes;
    std::filesystem::path sceneDir = GetSceneDirectory();
    if (!std::filesystem::exists(sceneDir)) return scenes;
    
    for (const auto& entry : std::filesystem::directory_iterator(sceneDir))
    {
        if (entry.path().extension() == ".scene")
            scenes.push_back(entry.path().stem().string());
    }
    return scenes;
}