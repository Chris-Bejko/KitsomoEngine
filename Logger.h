#pragma once
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>
#ifdef _WIN32
#include <windows.h>
#endif

enum class LogLevel
{
    Info,
    Warning,
    Error,
    Debug
};

class Logger
{
public:
    static Logger& get()
    {
        static Logger instance;
        return instance;
    }

    template<typename... Args>
    void Log(LogLevel level, Args&&... args)
    {
        std::ostringstream oss;
        (oss << ... << std::forward<Args>(args));
        Print(level, oss.str());
    }

    void SetLogToFile(bool value, std::string filename = "log.txt")
    {
        logToFile = value;
        if (logToFile)
            fileStream.open(filename, std::ios::app);
    }

private:
    Logger() = default;
    bool logToFile = false;
    std::ofstream fileStream;

    std::string GetPrefix(LogLevel level)
    {
        switch (level)
        {
        case LogLevel::Info:    return "[INFO]    ";
        case LogLevel::Warning: return "[WARNING] ";
        case LogLevel::Error:   return "[ERROR]   ";
        case LogLevel::Debug:   return "[DEBUG]   ";
        default:                return "[LOG]     ";
        }
    }

    std::string GetTimestamp()
    {
        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        char buf[20];
        struct tm timeInfo;
        localtime_s(&timeInfo, &t);
        strftime(buf, sizeof(buf), "%H:%M:%S", &timeInfo);
        return std::string(buf);
    }
    void SetConsoleColor(LogLevel level)
    {
#ifdef _WIN32
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        switch (level)
        {
        case LogLevel::Info:    SetConsoleTextAttribute(hConsole, 15); break; // White
        case LogLevel::Warning: SetConsoleTextAttribute(hConsole, 14); break; // Yellow
        case LogLevel::Error:   SetConsoleTextAttribute(hConsole, 12); break; // Red
        case LogLevel::Debug:   SetConsoleTextAttribute(hConsole, 11); break; // Cyan
        }
#endif
    }

    void ResetConsoleColor()
    {
#ifdef _WIN32
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
#endif
    }

    void Print(LogLevel level, const std::string& message)
    {
        std::string output = "[" + GetTimestamp() + "] " + GetPrefix(level) + message;
        SetConsoleColor(level);
        std::cout << output << std::endl;
        ResetConsoleColor();
        if (logToFile && fileStream.is_open())
            fileStream << output << "\n";
    }
};

// Convenience macros
#define LOG_INFO(...)    Logger::get().Log(LogLevel::Info,    __VA_ARGS__)
#define LOG_WARNING(...) Logger::get().Log(LogLevel::Warning, __VA_ARGS__)
#define LOG_ERROR(...)   Logger::get().Log(LogLevel::Error,   __VA_ARGS__)
#define LOG_DEBUG(...)   Logger::get().Log(LogLevel::Debug,   __VA_ARGS__)