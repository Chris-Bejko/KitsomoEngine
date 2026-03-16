#pragma once

#include <string>
#include "../AssetManager.h"
#include "../Component.h"
#include "../Vector2.h"
#include "Transform.h"
#include "../Color.h"
class Sprite : public Component
{
public:
    Sprite() = default;
    virtual ~Sprite() = default;
    Sprite(std::string textureId, int renderOrder = 0, Color color = Color());

    bool Init() override final;

    std::vector<SerializableVariable> *GetSerializedFields() override final;

    void Serialize();

    void InitSerializedFields(ReadableSerializableVariableMap map) override final;

    void draw() override final;
    void update(float dt) override final;

    int GetHeight();

    int GetWidth();

    void updateEngine(float dt) override final;

    sf::Vector2f GetScale();

    sf::FloatRect GetGlobalBounds();

    sf::FloatRect TranslateHitbox(sf::FloatRect &hitbox);

    sf::Vector2f GetPosition();

    sf::Vector2f GetOrigin();

    sf::Sprite GetSprite();
    
    bool GetForceRender() { return forceDrag; }


    void SetColor(const sf::Color &color);

    void SetColor(Color color);
    void SetOrigin(const Vector2F &origin);

    sf::Vector2f GetRotation();
    bool isMouseOver(const sf::Sprite &sprite, int mouseX, int mouseY);
    int RenderOrder();

    void SetRenderOrder(int i);
    void SetForceDrag(bool force) { forceDrag = force; }

private:
    std::string ColorID = "";
    int renderOrder;
    std::vector<SerializableVariable> variables;
    bool dragging = false;
    bool wasDragging = false; // Track previous frame's drag state
    sf::Vector2f mouseRectOffset;
    sf::Vector2f mousePos;
    Vector2F dragStartPosition; // Store position when drag starts

    int width = 0;
    int height = 0;
    float lastClickTime = 0.f;
    std::string textureID = "";
    sf::Sprite sprite;
    sf::Texture texture;
    float dragDelay = 0.075f;
    float dragTimer = 0.f;
    bool pendingDrag = false;

    std::string lastColorID = "";
    std::string lastTextureID = "";

    bool forceDrag = false;
};