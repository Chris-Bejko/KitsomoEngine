#include "ConsoleManager.h"

void ConsoleManager::AddToConsole(const std::string &message, LogLevel level)
{
	consoleLogs.push_back({message, level});
	if (consoleLogs.size() > 200)
		consoleLogs.pop_front();
}


ConsoleManager::ConsoleManager()
{
}

ConsoleManager::~ConsoleManager()
{
}

ConsoleManager& ConsoleManager::get()
{
    static ConsoleManager instance;
    return instance;
}