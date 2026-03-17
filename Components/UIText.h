#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include "../Component.h"
enum class TextAlignment { Left, Center, Right };
class UIText : public Component
{
public:
    UIText() = default;
    UIText(const std::string& text, const std::string& fontId, 
           unsigned int fontSize = 24);

    bool Init() override;
    void draw() override;
    void update(float dt) override;
    void updateEngine(float dt) override;

    void SetText(const std::string& text);
    void SetFontSize(unsigned int size);
    void SetColor(sf::Color color);
    void SetAlignment(TextAlignment align);
    void SetWordWrap(bool wrap) { wordWrap = wrap; }

    std::string GetText() { return text; }

    void Serialize() override;
    void InitSerializedFields(ReadableSerializableVariableMap map) override;
    std::vector<SerializableVariable>* GetSerializedFields() override { return &serializables; }

private:
    sf::Text sfText;
    std::string text = "Text";
    std::string fontId = "";
    std::string colorString = "White";
    unsigned int fontSize = 24;
    TextAlignment alignment = TextAlignment::Center;
    std::string alignmentString = "Center";
    bool wordWrap = false;
	std::vector<SerializableVariable> serializables;

    void UpdateTextProperties();
    void ApplyAlignment(sf::Vector2f screenPos);
};