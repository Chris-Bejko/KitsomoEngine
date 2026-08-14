#include "EventSystem.h"

EventSystem& EventSystem::get()
{
    static EventSystem instance;
    return instance;
}

void EventSystem::Clear()
{
    channels.clear();
}