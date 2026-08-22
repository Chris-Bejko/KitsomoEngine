// FileWatcher.h
#pragma once
#include <atomic>
#include <chrono>
#include <filesystem>
#include <functional>
#include <exception>
#include <map>
#include <thread>

#include "Logger.h"

class FileWatcher
{
public:
    ~FileWatcher()
    {
        Stop();
    }

    void Watch(const std::string& path, 
               std::function<void(std::string)> onChange)
    {
        Stop();
        watchPath    = path;
        callback     = onChange;
        running      = true;
        watchThread  = std::thread([this]() { Run(); });
    }

    void Stop()
    {
        running = false;
        if (watchThread.joinable())
            watchThread.join();
    }

private:
    void Run()
    {
        std::map<std::string, 
                 std::filesystem::file_time_type> lastWriteTimes;

        while (running)
        {
            try
            {
                if (!std::filesystem::exists(watchPath))
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                    continue;
                }

                for (auto& entry : std::filesystem::recursive_directory_iterator(watchPath))
                {
                    if (entry.path().extension() != ".cpp" &&
                        entry.path().extension() != ".h")
                    {
                        continue;
                    }

                    auto lastWrite = entry.last_write_time();
                    auto path      = entry.path().string();

                    if (lastWriteTimes.count(path) &&
                        lastWriteTimes[path] != lastWrite)
                    {
                        LOG_INFO("File changed: ", path.c_str());
                        callback(path);
                    }

                    lastWriteTimes[path] = lastWrite;
                }
            }
            catch (const std::exception &e)
            {
                LOG_WARNING("File watcher error: ", e.what());
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

    std::string                          watchPath;
    std::function<void(std::string)>     callback;
    std::thread                          watchThread;
    std::atomic<bool>                    running = false;
};