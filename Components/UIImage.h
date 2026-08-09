#pragma once
#include <SFML/Graphics.hpp>
#include "SerializableScript.h"
class Component;
class UIImage : public SerializableScript
{
public:
    UIImage() = default;
    UIImage(const std::string& textureId);

    bool Init() override;
    void draw() override;
    void update(float dt) override;
    void updateEngine(float dt) override;

    void SetTexture(const std::string& texId);
    void SetColor(sf::Color color);
    void SetAlpha(float alpha);

private:
    sf::Sprite uiSprite;
    sf::Texture uiTexture;
    std::string textureId = "";
    std::string colorString = "White";
    float alpha = 1.0f;

    void UpdateSprite();
};