#pragma once
#include "imgui.h"
#include <deque>
#include <string>
#include "Systems/include/System.h"
#include "Logger.h"

struct ConsoleEntry
{
    std::string message;
    LogLevel level;
};

class ConsoleManager : public System
{
    public:
        ConsoleManager();
        ~ConsoleManager();

        static ConsoleManager& get();

        void Clear() { consoleLogs.clear(); }
        void AddToConsole(const std::string& message, LogLevel level);

       const std::deque<ConsoleEntry>& GetConsoleLogs() const { return consoleLogs; }
    private:
         std::deque<ConsoleEntry> consoleLogs;

};