#pragma once
#include "EntityManager.h"
#include "InputSystem.h"
#include <SFML/Graphics.hpp>

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

    void Spawn(Entity* entity);

    size_t GetTotalEntities();

    void Save(std::string fileName);

    void Load(std::string fileName = "saveFile.txt");

    std::string GetSubstring(std::string & line, std::string & delStart, std::string & delEnd, bool erase);

    void Reset();

    inline static Engine& get()
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

    EntityManager* GetManager();


private:
    EngineState currentState;
    EngineState previousState;
    EntityManager* manager;
    bool isRunning;
    sf::RenderWindow *window;
    static Engine* s_instance;
    InputSystem* inputSystem;
    float dt = 1.f;

    bool entitiesAwaken;
};
