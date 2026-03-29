#pragma once
#include "../SerializableScript.h"
#include "../Vector2.h"
#include <SFML/Graphics.hpp>
#include <string>

enum class TextAlignment { Left, Center, Right };

class UIText : public SerializableScript
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

private:
    sf::Text      sfText;
    std::string   text           = "Text";
    std::string   fontId         = "";
    std::string   colorString    = "White";
    unsigned int  fontSize       = 24;
    TextAlignment alignment      = TextAlignment::Center;
    std::string   alignmentString = "Center";
    bool          wordWrap       = false;

    void UpdateTextProperties();
    void ApplyAlignment(Vector2F screenPos);
};