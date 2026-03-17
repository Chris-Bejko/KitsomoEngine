#pragma once
#include <SFML/Graphics.hpp>
#include "../Component.h"

class Component;
class UIImage : public Component
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

    void Serialize() override;
    void InitSerializedFields(ReadableSerializableVariableMap map) override;
    std::vector<SerializableVariable>* GetSerializedFields() override { return &serializables; }
private:
    sf::Sprite uiSprite;
    sf::Texture uiTexture;
    std::string textureId = "";
    std::string colorString = "White";
    float alpha = 1.0f;
	std::vector<SerializableVariable> serializables;

    void UpdateSprite();
};