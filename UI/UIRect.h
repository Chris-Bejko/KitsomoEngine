#pragma once
#include "../SerializableScript.h"
#include <SFML/Graphics.hpp>

enum class AnchorPreset
{
    TopLeft,
    TopCenter,
    TopRight,
    MiddleLeft,
    MiddleCenter,
    MiddleRight,
    BottomLeft,
    BottomCenter,
    BottomRight,
    StretchHorizontal,
    StretchVertical,
    StretchAll
};

class Canvas;

class UIRect : public SerializableScript
{
public:
    UIRect() = default;
    bool Init() override;

    void update(float dt) override;
    void updateEngine(float dt) override;
    // Position relative to anchor (replaces transform position for UI)
    sf::Vector2f anchorOffset = {0.f, 0.f};
    sf::Vector2f sizeDelta = {100.f, 50.f}; // width/height
    sf::Vector2f pivot = {0.5f, 0.5f};      // 0,0=topleft 1,1=bottomright
    AnchorPreset anchor = AnchorPreset::MiddleCenter;

    // Resolved screen rect
    sf::FloatRect GetScreenRect();
    sf::Vector2f GetScreenPosition();

    sf::Vector2f ResolveAnchor(AnchorPreset anchor, sf::Vector2f screenSize);
    Canvas *GetCanvas();
    void SyncToTransform();
private:
    std::string anchorString = "MiddleCenter";
    Canvas* cachedCanvas = nullptr;
};
