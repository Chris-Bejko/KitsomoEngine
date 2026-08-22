#pragma once

#include <filesystem>
#include <string>

class AssetReference
{
public:
    AssetReference() = default;
    explicit AssetReference(const std::string& path);
    virtual ~AssetReference() = default;

    AssetReference(const AssetReference&) = default;
    AssetReference& operator=(const AssetReference&) = default;

    AssetReference(AssetReference&&) noexcept = default;
    AssetReference& operator=(AssetReference&&) noexcept = default;

    void SetPath(const std::string& assetPath);

    const std::string& GetPath() const;

    bool HasPath() const;

    std::string GetName() const;


    // Runtime resource management.
    virtual bool Load() = 0;
    virtual void Unload() = 0;
    virtual bool IsLoaded() const = 0;

protected:
    static std::filesystem::path ResolvePath(
        const std::string& projectRelativePath);

    std::string path;
};