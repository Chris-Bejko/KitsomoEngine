#include "AssetReference.h"

#include <filesystem>

// TODO:
// Replace this with your actual project/runtime root resolver.
//
// The important architectural point is that AssetReference stores
// project-relative paths and resolves them only when loading.
//
// For now, paths are resolved relative to the current working directory.
std::filesystem::path AssetReference::ResolvePath(
    const std::string& projectRelativePath)
{
    if (projectRelativePath.empty())
        return {};

    std::filesystem::path path(projectRelativePath);

    if (path.is_absolute())
        return path;

    return std::filesystem::current_path() / path;
}

AssetReference::AssetReference(const std::string& path)
    : path(path)
{
}

void AssetReference::SetPath(const std::string& newPath)
{
    Unload();

    path = newPath;

    if (!path.empty())
        Load();
}

const std::string& AssetReference::GetPath() const
{
    return path;
}

std::string AssetReference::GetName() const
{
    if (path.empty())
        return {};

    return std::filesystem::path(path).filename().string();
}

bool AssetReference::HasPath() const
{
    return !path.empty();
}