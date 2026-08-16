#pragma once

#include "AssetReference.h"
#include <SFML/Audio.hpp>

class Audio : public AssetReference
{
public:
    Audio() = default;
    explicit Audio(const std::string& path);

    ~Audio() override;

    Audio(const Audio& other);
    Audio& operator=(const Audio& other);

    Audio(Audio&& other) noexcept;
    Audio& operator=(Audio&& other) noexcept;

    void SetPath(const std::string& path);

    bool Load() override;
    void Unload() override;
    bool IsLoaded() const override;

    sf::SoundBuffer* GetBuffer() const;

private:
    sf::SoundBuffer* buffer = nullptr;
};