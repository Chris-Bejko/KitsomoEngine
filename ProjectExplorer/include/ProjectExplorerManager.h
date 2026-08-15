#pragma once

#include "FileType.h"

#include <filesystem>
#include <memory>
#include <vector>

class ProjectExplorerManager
{
public:
    static ProjectExplorerManager& get();

    void SetDirectory(
        const std::filesystem::path& directory);

    const std::filesystem::path& GetDirectory() const;

    std::vector<std::filesystem::directory_entry>
    GetEntries() const;

    FileType* GetFileType(
        const std::filesystem::path& path);

private:
    ProjectExplorerManager();

    ProjectExplorerManager(
        const ProjectExplorerManager&) = delete;

    ProjectExplorerManager& operator=(
        const ProjectExplorerManager&) = delete;

    std::filesystem::path directory;

    std::vector<std::unique_ptr<FileType>>
        fileTypes;
};