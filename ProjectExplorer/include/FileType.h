#pragma once

#include <filesystem>

class FileType
{
public:
    virtual ~FileType() = default;

    virtual bool Supports(
        const std::filesystem::path& path) const = 0;

    virtual void Draw(
        const std::filesystem::path& path) = 0;
};