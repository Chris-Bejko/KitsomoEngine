#pragma once

#include "AssetReference.h"
#include <SFML/Graphics.hpp>

class Texture : public AssetReference
{
public:
    Texture() = default;
    explicit Texture(const std::string& path);

    ~Texture() override;

    Texture(const Texture& other);
    Texture& operator=(const Texture& other);

    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;

    bool Load() override;
    void Unload() override;
    bool IsLoaded() const override;

    sf::Texture* GetTexture() const;

    sf::Vector2u GetSize() const;

private:
    sf::Texture* texture = nullptr;
};