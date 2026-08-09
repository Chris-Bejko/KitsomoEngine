#pragma once

#include "System.h"
#include <map>
#include <string>

class InputSystem : public System
{
public:
    void Update() override final
    {
    }
    void SetKeyDown(const std::string id)
    {
        KeyDown[id] = !KeyPressed[id];
        KeyPressed[id] = true;
    }

    void SetKeyUp(const std::string id)
    {
        KeyUp[id] = KeyPressed[id];
        KeyPressed[id] = false;
    }

    bool GetKeyDown(const std::string id)
    {
        return KeyDown[id];
    }
    inline static InputSystem& get()
    {
        if (s_instance == nullptr)
        {
            s_instance = new InputSystem();
        }

        return *s_instance;
    }

private:
    static InputSystem* s_instance;

    std::map<std::string, bool> KeyPressed;

    std::map<std::string, bool> KeyDown;

    std::map<std::string, bool> KeyUp;
};