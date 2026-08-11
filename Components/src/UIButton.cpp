#include "UIButton.h"
#include "UIEventSystem.h"
#include "Canvas.h"
#include "Engine.h"
#include "imgui.h"
#include "UIText.h"
#include "UIImage.h"
#include "UIRect.h"
#include "ComponentRegistry.h"

DECLARE_COMPONENT_RULES(UIButton, false)
REGISTER_COMPONENT(UIButton)
REGISTER_SERIALIZABLE_COMPONENT(UIButton)

UIButton::UIButton()
{
}
UIButton::~UIButton()
{
    UIEventSystem::get().Unregister(this);
}

bool UIButton::Init()
{
    Field("interactable", interactable);
    Field("targetText", targetText);

    if (!entity->HasComponent<UIRect>())
        entity->AddComponent<UIRect>();

    if (!entity->HasComponent<UIImage>())
        entity->AddComponent<UIImage>();


    if(!targetText){

        Entity *textEntity = new Entity("Text");
        textEntity->AddComponent<UIText>();
        textEntity->SetParent(entity);
        Engine::get().Spawn(textEntity);
        targetText = textEntity;
    }
    Vector2F size = entity->GetComponent<UIRect>().sizeDelta;

    background.setSize(sf::Vector2f(size.x, size.y));
    background.setFillColor(normalColor);
    background.setOutlineColor(sf::Color(100, 100, 100, 200));
    background.setOutlineThickness(1.f);
    return true;
}

void UIButton::Awake()
{
    UIEventSystem::get().Register(this);
}

void UIButton::UpdateVisualState()
{
    if (!interactable)
    {
        background.setFillColor(disabledColor);
        return;
    }
    if (isPressed)
        background.setFillColor(pressedColor);
    else if (isHovered)
        background.setFillColor(hoverColor);
    else
        background.setFillColor(normalColor);
}

void UIButton::draw()
{
    Canvas *canvas = entity->GetComponent<UIRect>().GetCanvas();
    if (!canvas)
        return;

    sf::View prevView;
    bool screenSpace = canvas->GetRenderMode() == CanvasRenderMode::ScreenSpace;

    if (screenSpace)
    {
        prevView = Engine::get().GetWindow().getView();
        Engine::get().GetWindow().setView(Engine::get().GetWindow().getDefaultView());
    }

    Vector2F screenPos = entity->GetComponent<UIRect>().GetScreenPosition();
    Vector2F size = entity->GetComponent<UIRect>().sizeDelta;
    background.setSize(sf::Vector2f(size.x, size.y));
    background.setPosition(sf::Vector2f(screenPos.x, screenPos.y));
    UpdateVisualState();
    Engine::get().GetWindow().draw(background);

    if (screenSpace)
        Engine::get().GetWindow().setView(prevView);
}

void UIButton::update(float dt)
{
    // UIEventSystem handles input
}

void UIButton::updateEngine(float dt)
{
    draw(); // show in editor too
}

void UIButton::DrawEditorButton()
{
    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "Button Settings");
    ImGui::Checkbox("Interactable", &interactable);
}