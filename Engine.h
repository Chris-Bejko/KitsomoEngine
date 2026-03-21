#pragma once
#include "EntityManager.h"
#include "InputSystem.h"
#include <SFML/Graphics.hpp>
#include <functional>
#include "Vector2.h"

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

    inline static Engine &get()
    {
        if (s_instance == nullptr)
        {
            s_instance = new Engine();
        }

        return *s_instance;
    }

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

private:
    bool editorDragging = false;
    std::string openProject = "";
    sf::Vector2f editorDragStart;
    EngineState currentState;
    EngineState previousState;
    EntityManager *manager;
    bool isRunning;
    sf::RenderWindow *window;
    static Engine *s_instance;
    InputSystem *inputSystem;
    sf::Clock deltaClock;
    float dt = 1.f;
    std::string focusTargetName = "";
    float focusSpeed = 5.f;

    bool entitiesAwaken;

    std::string draggedEntity = "";
    std::unordered_map<std::string, ComponentFactory> componentRegistry;
    bool loading = false;
};
