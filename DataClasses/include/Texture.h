#pragma once

#include <filesystem>
#include <string>

class Texture
{
public:
    Texture() = default;

    Texture(const std::string& absolutePath)
    {
        SetPath(absolutePath);
    }

    Texture(const char* absolutePath)
    {
        SetPath(absolutePath ? absolutePath : "");
    }

    void SetPath(const std::string& absolutePath)
    {
        path = std::filesystem::path(absolutePath).lexically_normal().string();
        name = std::filesystem::path(path).filename().string();
    }

    const std::string& GetName() const
    {
        return name;
    }

    bool Empty() const
    {
        return path.empty();
    }

    bool Exists() const
    {
        return !path.empty() && std::filesystem::exists(path);
    }

    void Clear()
    {
        path.clear();
        name.clear();
    }

private:
    std::string path;
    std::string name;
};