// Logger.h
#pragma once
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>
#include <functional>
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

    // Register a callback to receive log messages
    using LogCallback = std::function<void(LogLevel, const std::string&)>;
    void SetCallback(LogCallback callback)
    {
        this->callback = callback;
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
    LogCallback callback;

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

    ImVec4 GetColor(LogLevel level)
    {
        switch (level)
        {
        case LogLevel::Info:    return ImVec4(0.9f, 0.9f, 0.9f, 1.0f);
        case LogLevel::Warning: return ImVec4(0.95f, 0.78f, 0.2f, 1.0f);
        case LogLevel::Error:   return ImVec4(1.0f, 0.3f, 0.3f, 1.0f);
        case LogLevel::Debug:   return ImVec4(0.4f, 0.85f, 1.0f, 1.0f);
        default:                return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
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
        case LogLevel::Info:    SetConsoleTextAttribute(hConsole, 15); break;
        case LogLevel::Warning: SetConsoleTextAttribute(hConsole, 14); break;
        case LogLevel::Error:   SetConsoleTextAttribute(hConsole, 12); break;
        case LogLevel::Debug:   SetConsoleTextAttribute(hConsole, 11); break;
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
        // Fire callback if registered
        if (callback)
            callback(level, output);
    }
};

#define LOG_INFO(...)    Logger::get().Log(LogLevel::Info,    __VA_ARGS__)
#define LOG_WARNING(...) Logger::get().Log(LogLevel::Warning, __VA_ARGS__)
#define LOG_ERROR(...)   Logger::get().Log(LogLevel::Error,   __VA_ARGS__)
#define LOG_DEBUG(...)   Logger::get().Log(LogLevel::Debug,   __VA_ARGS__)