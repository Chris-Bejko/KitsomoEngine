#pragma once

#include <string>
#include <deque>
#include "imgui.h"

class StatusManager
{
public:
    struct Status
    {
        std::string message;
        ImVec4 color;
        float lifetime;
        float maxLifetime;
    };

    static StatusManager& get();

    void Notify(const std::string& message, ImVec4 color, float duration = 2.5f);
    void Update(float dt);
    void Clear();

    const std::deque<Status>& GetStatuses() const { return statuses; }

private:
    StatusManager() = default;

    std::deque<Status> statuses;
};