#include "StatusManager.h"
#include <algorithm>

StatusManager& StatusManager::get()
{
    static StatusManager instance;
    return instance;
}

void StatusManager::Notify(const std::string& message, ImVec4 color, float lifetime)
{
    statuses.push_back({message, color, lifetime, lifetime});
}

void StatusManager::Update(float dt)
{
    for (auto& status : statuses)
        status.lifetime -= dt;

    statuses.erase(std::remove_if(statuses.begin(), statuses.end(), [](const Status& status) { return status.lifetime <= 0.0f; }), statuses.end());
}

void StatusManager::Clear()
{
    statuses.clear();
}