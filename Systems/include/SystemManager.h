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

    static SystemsManager& get();

private:
    std::vector<std::unique_ptr<System>> systems;
    static SystemsManager* s_instance;
};