#pragma once

#include "FileType.h"

class TextureFile : public FileType
{
public:
    bool Supports(
        const std::filesystem::path& path) const override;

    void Draw(
        const std::filesystem::path& path) override;
};