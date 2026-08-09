#include "SceneManager.h"
#include "Engine.h"
#include "Logger.h"
#include <filesystem>
#include <algorithm>

void SceneManager::Init()
{
    if (!HasProjectRoot())
    {
        LOG_INFO("SceneManager initialized without an active project");
        return;
    }

    std::filesystem::create_directories(GetSceneDirectory());
    std::filesystem::create_directories(GetPrefabDirectory());
    LOG_INFO("SceneManager initialized");
}

void SceneManager::LoadScene(const std::string& sceneName, SceneLoadMode mode)
{
    if (!HasProjectRoot())
    {
        LOG_WARNING("Cannot load scene without a project");
        return;
    }

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
    if (!HasProjectRoot())
    {
        LOG_WARNING("Cannot save scene without a project");
        return;
    }

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

bool SceneManager::OpenProject(const std::string& path)
{
    if (path.empty())
        return false;

    std::filesystem::path root = std::filesystem::absolute(path);
    if (!std::filesystem::exists(root) || !std::filesystem::is_directory(root))
        return false;

    SetProjectRoot(root.string());
    currentScene.clear();
    loadedScenes.clear();
    LOG_INFO("Opened project: ", root.string().c_str());
    return true;
}

void SceneManager::ClearProjectRoot()
{
    projectRoot.clear();
    currentScene.clear();
    loadedScenes.clear();
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
    return projectRoot;
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
    if (!HasProjectRoot())
        return std::filesystem::path();

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

std::vector<std::filesystem::path> SceneManager::GetAvailableProjects(const std::string& baseDirectory) const
{
    std::vector<std::filesystem::path> projects;
    std::filesystem::path base = std::filesystem::absolute(baseDirectory.empty() ? "Projects" : baseDirectory);
    if (!std::filesystem::exists(base))
        return projects;

    for (const auto& entry : std::filesystem::directory_iterator(base))
    {
        if (!entry.is_directory())
            continue;

        auto assetsDir = entry.path() / "Assets";
        if (std::filesystem::exists(assetsDir) && std::filesystem::is_directory(assetsDir))
            projects.push_back(entry.path());
    }

    std::sort(projects.begin(), projects.end());
    return projects;
}

std::vector<std::string> SceneManager::GetAvailableScenes()
{
    std::vector<std::string> scenes;
    if (!HasProjectRoot())
        return scenes;

    std::filesystem::path sceneDir = GetSceneDirectory();
    if (!std::filesystem::exists(sceneDir)) return scenes;
    
    for (const auto& entry : std::filesystem::directory_iterator(sceneDir))
    {
        if (entry.path().extension() == ".scene")
            scenes.push_back(entry.path().stem().string());
    }
    return scenes;
}