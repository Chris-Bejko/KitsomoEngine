#pragma once

#include "AssetManager.h"
#include "Color.h"
#include "Component.h"
#include "SerializableScript.h"
#include "Texture.h"
#include "Transform.h"
#include "Vector2.h"

#include <SFML/Graphics.hpp>
#include <string>

class Sprite : public SerializableScript
{
public:
    Sprite() = default;
    virtual ~Sprite() = default;

    Sprite(std::string texturePath, int renderOrder = 0, Color color = Color());

    bool Init() override final;

    void draw() override final;
    void update(float dt) override final;
    void updateEngine(float dt) override final;

    int GetHeight();
    int GetWidth();

    sf::Vector2f GetScale();
    sf::FloatRect GetGlobalBounds();
    sf::FloatRect TranslateHitbox(sf::FloatRect& hitbox);
    sf::Vector2f GetPosition();
    sf::Vector2f GetOrigin();
    sf::Sprite GetSprite();

    bool GetForceRender() { return forceDrag; }

    void SetColor(const sf::Color& color);
    void SetColor(Color color);
    void SetOrigin(const Vector2F& origin);

    sf::Vector2f GetRotation();

    bool isMouseOver(
        const sf::Sprite& sprite,
        int mouseX,
        int mouseY);

    int RenderOrder();
    void SetRenderOrder(int i);

    void SetForceDrag(bool force)
    {
        forceDrag = force;
    }

    void OnFieldChanged(
        const std::string& fieldName) override;

private:
    void UpdateSprite();
    void RefreshColor();

private:

    Texture _texture;

    std::string ColorID = "";

    int renderOrder = 0;

    sf::Sprite sprite;
    sf::Texture* texture;

    int width = 0;
    int height = 0;

    float lastClickTime = 0.f;

    float dragDelay = 0.075f;
    float dragTimer = 0.f;

    bool dragging = false;
    bool wasDragging = false;
    bool pendingDrag = false;
    bool forceDrag = false;

    sf::Vector2f mouseRectOffset;
    sf::Vector2f mousePos;

    Vector2F dragStartPosition;

    std::string lastColorID = "";
    std::string lastTexturePath = "";
};