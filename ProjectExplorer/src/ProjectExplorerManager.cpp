#include "ProjectExplorerManager.h"

#include "TextureFile.h"
#include "FontFile.h"
#include "SceneFile.h"
#include "ScriptFile.h"
#include "PrefabFile.h"
#include "AudioFile.h"

#include <algorithm>

ProjectExplorerManager::ProjectExplorerManager()
{
    fileTypes.push_back(
        std::make_unique<TextureFile>());

    fileTypes.push_back(
        std::make_unique<FontFile>());

    fileTypes.push_back(
        std::make_unique<SceneFile>());

    fileTypes.push_back(
        std::make_unique<ScriptFile>());

    fileTypes.push_back(
        std::make_unique<PrefabFile>());

    fileTypes.push_back(
        std::make_unique<AudioFile>());
}

ProjectExplorerManager&
ProjectExplorerManager::get()
{
    static ProjectExplorerManager instance;
    return instance;
}

void ProjectExplorerManager::SetDirectory(
    const std::filesystem::path& newDirectory)
{
    directory = newDirectory;
}

const std::filesystem::path&
ProjectExplorerManager::GetDirectory() const
{
    return directory;
}

std::vector<std::filesystem::directory_entry>
ProjectExplorerManager::GetEntries() const
{
    std::vector<std::filesystem::directory_entry>
        entries;

    if (directory.empty() ||
        !std::filesystem::exists(directory))
    {
        return entries;
    }

    for (const auto& entry :
         std::filesystem::directory_iterator(directory))
    {
        entries.push_back(entry);
    }

    std::sort(
        entries.begin(),
        entries.end(),
        [](const auto& a, const auto& b)
        {
            const bool aDirectory =
                a.is_directory();

            const bool bDirectory =
                b.is_directory();

            if (aDirectory != bDirectory)
                return aDirectory > bDirectory;

            return a.path().filename().string() <
                   b.path().filename().string();
        });

    return entries;
}

FileType*
ProjectExplorerManager::GetFileType(
    const std::filesystem::path& path)
{
    for (auto& fileType : fileTypes)
    {
        if (fileType->Supports(path))
            return fileType.get();
    }

    return nullptr;
}