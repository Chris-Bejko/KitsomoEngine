#pragma once

#include <SFML/Graphics.hpp>
#include "SerializableScript.h"
#include "Texture.h"

class Component;

class UIImage : public SerializableScript
{
public:
    UIImage() = default;
    UIImage(const std::string& texturePath);

    bool Init() override;
    void draw() override;
    void update(float dt) override;
    void updateEngine(float dt) override;

    void SetTexture(const std::string& texturePath);
    void SetColor(sf::Color color);
    void SetAlpha(float alpha);

private:
    sf::Sprite uiSprite;
    sf::Texture* uiTexture = nullptr;

    Texture _texture;

    std::string colorString = "White";
    float alpha = 1.0f;

    void UpdateSprite();
};