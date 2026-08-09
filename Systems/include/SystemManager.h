#pragma once

#include "System.h"
#include <vector>
#include <memory>

class SystemsManager
{
public:
    SystemsManager() = default;
    ~SystemsManager() = default;
    void AddSystem(System* system);
    void Update();

    inline static SystemsManager& get()
    {
        if (s_instance == nullptr)
        {
            s_instance = new SystemsManager();
        }

        return *s_instance;
    }

private:
    std::vector<std::unique_ptr<System>> systems;
    static SystemsManager* s_instance;
};