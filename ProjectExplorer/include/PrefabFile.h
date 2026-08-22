#pragma once

#include "FileType.h"

class PrefabFile : public FileType
{
public:
    bool Supports(
        const std::filesystem::path& path) const override;

    void Draw(
        const std::filesystem::path& path) override;
};