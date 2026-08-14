#pragma once
#include "EntityManager.h"
#include "InputSystem.h"
#include <SFML/Graphics.hpp>
#include <functional>
#include <memory>
#include "Vector2.h"
#include "Events.h"
class ProjectModuleLoader;
class Sprite;
using ComponentFactory = std::function<void(Entity *, ReadableSerializableVariableMap, std::string)>;

constexpr int SCREEN_WIDTH = 1280;
constexpr int SCREEN_HEIGHT = 720;
enum EngineState
{
    Running,
    PlayMode,
    Paused
};

class Engine
{
public:
    bool isEngine;
    bool pendingRecompile = false;

    Engine();
    ~Engine();

    void Quit();
    void Init();
    void Clean();

    void Render();

    void Update();

    void Events();

    void Spawn(Entity *entity);

    size_t GetTotalEntities();

    void Save(const std::string &fileName);

    bool Load(std::string fileName = "saveFile.txt");

    void Reset();

    void RemoveEntity(Entity *entity);

    static Engine &get();
    
    EngineState GetCurrentState();

    void SetEngineState(EngineState engineState);

    bool IsRunning();

    sf::RenderWindow &GetWindow();

    EntityManager *GetManager();

    bool DraggingEntity();

    std::string GetDraggedEntity();

    void TriggerDragging(std::string newDragged);

    void ClearInpsector();

    void SetView(sf::View &view);

    sf::FloatRect GetView();

    void SavePrefab(Entity *entity);
    bool LoadPrefab(std::string prefabName);
    float GetDt() { return dt; }
    void UpdateEditorCamera(float dt);
    void RegisterComponents();
    void SpawnEntities(const std::vector<SerializableEntity> &entities);
    std::vector<SerializableEntity> ParseFile(const std::string &fileName);

    Entity *SpawnPrefab(const std::string prefabName, Vector2F position);
    void FocusOnEntity(Entity *entity);
    bool IsLoading() { return loading; }
    void TriggerGameOver();
    void ProcessDestroyQueue();
    void QueueDestroyChildren(Entity *e);

    void QueueDestroy(const std::string& guid);
    bool OpenProject(const std::string &projectPath);
    void OpenProject(const OpenProjectEvent& event);
    bool ReloadProjectScripts();
    void RequestScriptRecompile();
    void PrepareForProjectModuleUnload();
    void Draw(Sprite *sprite);
private:
    void ProcessHotReloading();

    bool editorDragging = false;
    std::string openProject = "";
    sf::Vector2f editorDragStart;
    EngineState currentState;
    EngineState previousState;
    EntityManager *manager;
    bool isRunning;
    sf::RenderWindow *window;
    InputSystem *inputSystem;
    sf::Clock deltaClock;
    float dt = 1.f;
    std::vector<std::string> destroyQueue;
    std::string focusTargetName = "";
    float focusSpeed = 5.f;

    bool entitiesAwaken;

    std::string draggedEntity = "";
    std::unordered_map<std::string, ComponentFactory> componentRegistry;
    bool loading = false;
    bool isGameOver = false;
    bool recompileRequested = false;
    std::unique_ptr<ProjectModuleLoader> projectModuleLoader;

    void SubscribeEvents();
};
