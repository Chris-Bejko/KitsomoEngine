#pragma once
#include <SFML/Graphics.hpp>
#include "../SerializableScript.h"

enum class CanvasRenderMode
{
    ScreenSpace,
    WorldSpace
};

enum class CanvasScaleMode
{
    ConstantPixelSize,
    ScaleWithScreenSize,
};

class Canvas : public SerializableScript
{
public:
    Canvas() = default;
    virtual ~Canvas() = default;

    bool Init() override;
    void draw() override;
    void update(float dt) override;
    void updateEngine(float dt) override;

    CanvasRenderMode GetRenderMode() { return renderMode; }
    void SetRenderMode(CanvasRenderMode mode) { renderMode = mode; }

    void DrawEditorButton() override;
    void OnFieldChanged(const std::string &fieldName) override
    {
        if (fieldName == "renderMode")
            renderMode = (renderModeString == "WorldSpace") ? CanvasRenderMode::WorldSpace : CanvasRenderMode::ScreenSpace;
    }

    void UpdateScaleFactor();

    float GetScaleFactor() {return scaleFactor;}
private:
    CanvasRenderMode renderMode = CanvasRenderMode::ScreenSpace;
    std::string renderModeString = "ScreenSpace";
    CanvasScaleMode scaleMode = CanvasScaleMode::ScaleWithScreenSize;
    std::string scaleModeString = "ScaleWithScreenSize";
    Vector2F referenceResolution = Vector2F(1280.f, 720.f);
    float scaleFactor = 1.0f;
};