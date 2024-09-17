#pragma once
#include "Engine.h"
#include "../EntityManager.h"
#include "../InputSystem.h"

constexpr int SCREEN_WIDTH = 1280;
constexpr int SCREEN_HEIGHT = 720;

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

    inline static Engine& get()
    {
        if (s_instance == nullptr)
        {
            s_instance = new Engine();
        }

        return *s_instance;
    }

    inline bool IsRunning()
    {
        return isRunning;
    }
    sf::RenderWindow &GetWindow();
private:
    EntityManager* manager;
    bool isRunning;
    sf::RenderWindow *window;
    static Engine* s_instance;
    InputSystem* inputSystem;
};
