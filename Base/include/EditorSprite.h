#pragma once
#include "Component.h"
#include <SFML/Graphics.hpp>
#include "AssetManager.h"
#include "Sprite.h"

class EditorSprite : public Component
{
public:
    EditorSprite() = default;
    EditorSprite(const std::string &textureId);

    bool Init() override;
    void draw() override;
    void updateEngine(float dt) override;

    sf::FloatRect GetGlobalBounds() { return sprite.getGlobalBounds(); }
    bool IsMouseOver(const sf::Sprite &_sprite, sf::Vector2f mousePos)
    {
        return _sprite.getGlobalBounds().contains(mousePos);
    }

    std::string textureId = "editor_entity"; // default editor icon

private:
    sf::Sprite sprite;
    sf::Texture texture;
    bool dragging = false;
    bool pendingDrag = false;
    float dragTimer = 0.f;
    float dragDelay = 0.15f;
    float lastClickTime = 0.f;
    sf::Vector2f mouseRectOffset;
    bool forceDrag = false;
    sf::Vector2f dragStartAnchorOffset;
    sf::Vector2f dragStartMousePos;
};