#pragma once
#include <SFML/Graphics.hpp>
#include "../SerializableScript.h"

enum class CanvasRenderMode
{
    ScreenSpace,
    WorldSpace
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

    sf::Vector2f referenceResolution = {1280.f, 720.f};

    void DrawEditorButton() override;
    void OnFieldChanged(const std::string &fieldName) override
    {
        if (fieldName == "renderMode")
            renderMode = (renderModeString == "WorldSpace") ? CanvasRenderMode::WorldSpace : CanvasRenderMode::ScreenSpace;
    }

private:
    CanvasRenderMode renderMode = CanvasRenderMode::ScreenSpace;
    std::string renderModeString = "ScreenSpace";
};