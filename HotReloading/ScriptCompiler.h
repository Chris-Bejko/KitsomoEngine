#pragma once

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono>
#include <iomanip>

#include "Logger.h"

class ScriptCompiler
{
public:
    inline static std::string lastError = "";
    inline static bool lastCompileFailed = false;
    inline static std::string enginePath = "";

    static bool GenerateAndCompile(const std::string& projectPath)
    {
        const std::string resolvedEnginePath = GetEnginePath();
        const std::filesystem::path projectRoot =
            std::filesystem::absolute(projectPath);

        lastCompileFailed = false;
        lastError.clear();

        // ------------------------------------------------------------
        // Build log directory
        // ------------------------------------------------------------

        const std::filesystem::path logDirectory =
            projectRoot / "Generated" / "BuildLogs";

        std::error_code ec;
        std::filesystem::create_directories(logDirectory, ec);

        if (ec)
        {
            LOG_ERROR(
                "Failed to create build log directory: ",
                logDirectory.string().c_str());

            lastCompileFailed = true;
            lastError = "Failed to create build log directory";
            return false;
        }

        const std::string timestamp = GetTimestamp();

        const std::filesystem::path logFile =
            logDirectory /
            ("GameScripts_build_" + timestamp + ".log");

        LOG_INFO(
            "GameScripts build log: ",
            logFile.string().c_str());

        // ------------------------------------------------------------
        // Helper for running commands
        // ------------------------------------------------------------

        auto RunCommand =
            [&](const std::string& command,
                const std::string& description) -> bool
        {
            LOG_INFO(description.c_str());

            AppendLog(
                logFile,
                "\n============================================================\n"
                + description +
                "\n============================================================\n"
                "COMMAND:\n" +
                command +
                "\n\nOUTPUT:\n");

            // Redirect stdout and stderr into the same log.
            //
            // 2>&1 means:
            // stderr -> stdout
            // stdout -> log
            //
            // `>>` appends instead of overwriting.
            const std::string loggedCommand =
                command +
                " >> \"" +
                logFile.string() +
                "\" 2>&1";

            const int result = std::system(loggedCommand.c_str());

            AppendLog(
                logFile,
                "\nEXIT CODE: " +
                std::to_string(result) +
                "\n");

            if (result != 0)
            {
                lastCompileFailed = true;

                lastError =
                    description +
                    " failed. See build log: " +
                    logFile.string();

                LOG_ERROR(
                    description.c_str(),
                    " FAILED. See: ",
                    logFile.string().c_str());

                return false;
            }

            return true;
        };

        // ------------------------------------------------------------
        // 1. Generate registry
        // ------------------------------------------------------------

        std::string genCmd =
            "python \"" +
            resolvedEnginePath +
            "/Tools/GenerateRegistry.py\" \"" +
            projectRoot.string() +
            "\" \"" +
            resolvedEnginePath +
            "\"";

        if (!RunCommand(
                genCmd,
                "Generating component registry..."))
        {
            return false;
        }

        // ------------------------------------------------------------
        // 2. Configure CMake
        // ------------------------------------------------------------

        std::string configureCmd =
            "cmake "
            "-S \"" +
            (projectRoot / "Generated").string() +
            "\" "
            "-B \"" +
            (projectRoot / "build").string() +
            "\" "
            "-DENGINE_ROOT=\"" +
            resolvedEnginePath +
            "\" "
            "-DPROJECT_ROOT=\"" +
            projectRoot.string() +
            "\"";

        if (!RunCommand(
                configureCmd,
                "Configuring script module..."))
        {
            return false;
        }

        // ------------------------------------------------------------
        // 3. Build GameScripts
        // ------------------------------------------------------------

        std::string buildCmd =
            "cmake --build \"" +
            (projectRoot / "build").string() +
            "\" "
            "--target GameScripts "
            "--config Debug";

        if (!RunCommand(
                buildCmd,
                "Compiling scripts..."))
        {
            return false;
        }

        // ------------------------------------------------------------
        // Success
        // ------------------------------------------------------------

        LOG_INFO("Scripts compiled successfully!");

        AppendLog(
            logFile,
            "\n============================================================\n"
            "BUILD SUCCESSFUL\n"
            "============================================================\n");

        return true;
    }

private:

    static std::string GetEnginePath()
    {
        if (!enginePath.empty())
        {
            return enginePath;
        }

        enginePath =
            std::filesystem::current_path().string();

        return enginePath;
    }

    static std::string GetTimestamp()
    {
        const auto now =
            std::chrono::system_clock::now();

        const std::time_t time =
            std::chrono::system_clock::to_time_t(now);

        std::tm localTime{};

#ifdef _WIN32
        localtime_s(&localTime, &time);
#else
        localtime_r(&time, &localTime);
#endif

        std::ostringstream stream;

        stream << std::put_time(
            &localTime,
            "%Y%m%d_%H%M%S");

        return stream.str();
    }

    static void AppendLog(
        const std::filesystem::path& file,
        const std::string& text)
    {
        std::ofstream output(
            file,
            std::ios::app);

        if (!output)
        {
            return;
        }

        output << text;
        output.flush();
    }
};