#pragma once
#include "../Component.h"
#include <SFML/Graphics.hpp>

enum class CanvasRenderMode { ScreenSpace, WorldSpace };

class Canvas : public Component
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

    void Serialize() override;
    std::vector<SerializableVariable>* GetSerializedFields() override { return &serializables; }
    void InitSerializedFields(ReadableSerializableVariableMap map) override;

    void DrawEditorButton() override;

private:
    CanvasRenderMode renderMode = CanvasRenderMode::ScreenSpace;
    std::string renderModeString = "ScreenSpace";
    std::vector<SerializableVariable> serializables;
};