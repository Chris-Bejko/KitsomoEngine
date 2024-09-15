#include "SystemManager.h"

SystemsManager* SystemsManager::s_instance = nullptr;

void SystemsManager::AddSystem(System* system)
{
    std::unique_ptr<System> uniquePtr{ system };

    systems.emplace_back(std::move(uniquePtr));
}

void SystemsManager::Update()
{
    for (auto& system : systems)
    {
        system->Update();
    }
}