#include "UIRect.h"
#include "Canvas.h"
#include "Engine.h"
#include "Transform.h"
#include "GizmoSystem.h"
#include "Logger.h"
#include "ComponentRegistry.h"

REGISTER_SERIALIZABLE_COMPONENT(UIRect, false)

bool UIRect::Init()
{
    if (entity->HasComponent<Transform>())
        entity->GetComponent<Transform>().isUITransform = true;

    Field("anchorOffset", anchorOffset);
    Field("sizeDelta", sizeDelta);
    Field("pivot", pivot);
    return true;
}

Canvas *UIRect::GetCanvas()
{
    if (cachedCanvas != nullptr)
        return cachedCanvas;

    Entity *current = entity->GetParent();
    while (current != nullptr)
    {
        if (current->HasComponent<Canvas>())
        {
            cachedCanvas = &current->GetComponent<Canvas>();
            return cachedCanvas;
        }
        current = current->GetParent();
    }
    return nullptr;
}

void UIRect::SyncToTransform()
{
    if (!entity->HasComponent<Transform>())
        return;

    Vector2F screenPos = GetScreenPosition();
    entity->transform->position = screenPos;
}

Vector2F UIRect::ResolveAnchor(AnchorPreset anchor, Vector2F screenSize)
{
    switch (anchor)
    {
    case AnchorPreset::TopLeft:
        return Vector2F(0, 0);
    case AnchorPreset::TopCenter:
        return Vector2F(screenSize.x * 0.5f, 0);
    case AnchorPreset::TopRight:
        return Vector2F(screenSize.x, 0);
    case AnchorPreset::MiddleLeft:
        return Vector2F(0, screenSize.y * 0.5f);
    case AnchorPreset::MiddleCenter:
        return Vector2F(screenSize.x * 0.5f, screenSize.y * 0.5f);
    case AnchorPreset::MiddleRight:
        return Vector2F(screenSize.x, screenSize.y * 0.5f);
    case AnchorPreset::BottomLeft:
        return Vector2F(0, screenSize.y);
    case AnchorPreset::BottomCenter:
        return Vector2F(screenSize.x * 0.5f, screenSize.y);
    case AnchorPreset::BottomRight:
        return Vector2F(screenSize.x, screenSize.y);
    default:
        return Vector2F(screenSize.x * 0.5f, screenSize.y * 0.5f);
    }
}

Vector2F UIRect::GetScreenPosition()
{
    Canvas* canvas = GetCanvas();
    if (!canvas) return entity->transform->GetWorldPosition();

    canvas->UpdateScaleFactor();
    float scale = canvas->GetScaleFactor();

    auto windowSize = Engine::get().GetWindow().getSize();
    Vector2F screenSize((float)windowSize.x, (float)windowSize.y);

    if (entity->GetParent() && entity->GetParent()->HasComponent<UIRect>())
    {
        auto& parentRect = entity->GetParent()->GetComponent<UIRect>();
        sf::FloatRect parentScreenRect = parentRect.GetScreenRect();

        Vector2F parentSize(parentScreenRect.width, parentScreenRect.height);
        Vector2F parentAnchorPos = ResolveAnchor(anchor, parentSize);
        Vector2F pivotOffset(sizeDelta.x * scale * pivot.x, 
                             sizeDelta.y * scale * pivot.y);

        return Vector2F(
            parentScreenRect.left + parentAnchorPos.x + anchorOffset.x * scale - pivotOffset.x,
            parentScreenRect.top  + parentAnchorPos.y + anchorOffset.y * scale - pivotOffset.y
        );
    }

    Vector2F anchorPos   = ResolveAnchor(anchor, screenSize);
    Vector2F pivotOffset(sizeDelta.x * scale * pivot.x, 
                         sizeDelta.y * scale * pivot.y);
    return anchorPos + Vector2F(anchorOffset.x * scale, anchorOffset.y * scale) - pivotOffset;
}

sf::FloatRect UIRect::GetScreenRect()
{
    Canvas* canvas = GetCanvas();
    float scale = canvas ? canvas->GetScaleFactor() : 1.0f;
    Vector2F pos = GetScreenPosition();
    return sf::FloatRect(pos.x, pos.y, sizeDelta.x * scale, sizeDelta.y * scale);
}

void UIRect::updateEngine(float dt)
{
    SyncToTransform();
}

void UIRect::update(float dt)
{
    SyncToTransform();
}