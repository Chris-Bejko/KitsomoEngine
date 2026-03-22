#pragma once
#include "../SerializableScript.h"
#include <SFML/Graphics.hpp>
#include "../Vector2.h"
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
    Vector2F anchorOffset = {0.f, 0.f};
    Vector2F sizeDelta = {100.f, 50.f}; // width/height
    Vector2F pivot = {0.5f, 0.5f};      // 0,0=topleft 1,1=bottomright
    AnchorPreset anchor = AnchorPreset::MiddleCenter;

    // Resolved screen rect
    sf::FloatRect GetScreenRect();
    Vector2F GetScreenPosition();

    Vector2F ResolveAnchor(AnchorPreset anchor, Vector2F screenSize);
    Canvas *GetCanvas();
    void SyncToTransform();
private:
    std::string anchorString = "MiddleCenter";
    Canvas* cachedCanvas = nullptr;
};
