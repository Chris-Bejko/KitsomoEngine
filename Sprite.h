#pragma once

#include <string>
#include "../Entity.h"
#include "../AssetManager.h"
#include "../RenderContainer.h"
#include "../Engine.h"
#include "SFML/Graphics.hpp"

class Sprite : public Component
{
public:
    Sprite() = default;
    virtual ~Sprite() = default;
    Sprite(std::string textureId)
    {
        textureID = textureId;
    }
    inline bool Init() override final
    {
        transform = &entity->GetComponent<Transform>();
        texture = AssetManager::get().getTexture(textureID);
        sprite.setTexture(texture);

        return true;
    }

    inline void draw() override final
    {
        Engine::get().GetWindow().draw(sprite);
    }

    inline void update() override final
    {
        sprite.setPosition(transform->position.x, transform->position.y);
        sprite.setRotation(transform->rotation);
    }

    inline int GetHeight()
    {
        return height;
    }

    inline int GetWidth()
    {
        return width;
    }

    inline sf::FloatRect GetGlobalBounds()
    {
        return sprite.getGlobalBounds();
    }

    inline sf::FloatRect TranslateHitbox(sf::FloatRect& hitbox)
    {
        return sprite.getTransform().transformRect(hitbox);
    }
private:
    int width = 0;
    int height = 0;

    Transform* transform = nullptr;
    std::string textureID = "";
    sf::Sprite sprite;
    sf::Texture texture;
};