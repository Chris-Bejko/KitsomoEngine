#pragma once

#include <filesystem>
#include <string>

class ProjectExporter
{
public:
    struct Settings
    {
        std::string gameName = "Game";
        std::string executableName = "Game";
        std::string startupScene = "MainNew";

        std::filesystem::path outputDirectory;

        bool copyAssets = true;
        bool copyOpenAL = true;
    };

    static bool Export(
        const std::filesystem::path& projectRoot,
        const Settings& settings);

private:
    static bool GenerateRuntimeFiles(
        const std::filesystem::path& projectRoot,
        const std::filesystem::path& exportRoot,
        const Settings& settings);

    static bool WriteRuntimeMain(
        const std::filesystem::path& exportRoot,
        const Settings& settings);

    static bool WriteRuntimeCMake(
        const std::filesystem::path& projectRoot,
        const std::filesystem::path& exportRoot,
        const Settings& settings);

    static bool CopyProjectAssets(
        const std::filesystem::path& projectRoot,
        const std::filesystem::path& exportRoot);

    static bool CopyRuntimeDependencies(
        const std::filesystem::path& projectRoot,
        const std::filesystem::path& exportRoot,
        const Settings& settings);

    static bool CopyGameScripts(
        const std::filesystem::path& projectRoot,
        const std::filesystem::path& exportRoot);

    static bool BuildExport(
        const std::filesystem::path& exportRoot,
        const Settings& settings);

    static bool CopyFileIfExists(
        const std::filesystem::path& source,
        const std::filesystem::path& destination);

    static bool RunCommand(
        const std::string& command,
        const std::filesystem::path& workingDirectory);

    static std::string EscapeCMake(
        const std::string& value);
};