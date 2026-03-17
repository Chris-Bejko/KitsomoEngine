#include "UIButton.h"
#include "UIEventSystem.h"
#include "Canvas.h"
#include "../Engine.h"
#include "imgui.h"
#include "UIText.h"
#include "UIImage.h"
#include "../UI/UIRect.h"

UIButton::UIButton()
{
}
UIButton::~UIButton()
{
    UIEventSystem::get().Unregister(this);
}

bool UIButton::Init()
{
    Serialize();

    sf::Vector2f size = entity->GetComponent<UIRect>().sizeDelta;
    if (!entity->HasComponent<UIRect>())
        entity->AddComponent<UIRect>();

    Entity *imageEntity = new Entity("Image");
    imageEntity->AddComponent<UIImage>();
    imageEntity->SetParent(entity);
    Engine::get().Spawn(imageEntity);

    Entity *textEntity = new Entity("Text");
    textEntity->AddComponent<UIText>();
    textEntity->SetParent(entity);
    Engine::get().Spawn(textEntity);
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

    sf::Vector2f screenPos = entity->GetComponent<UIRect>().GetScreenPosition();
    sf::Vector2f size = entity->GetComponent<UIRect>().sizeDelta;
    background.setSize(sf::Vector2f(size.x, size.y));
    background.setPosition(screenPos);
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

void UIButton::Serialize()
{
    serializables.push_back({"interactable", &interactable, bool_Type});
}

void UIButton::InitSerializedFields(ReadableSerializableVariableMap map)
{
    for (auto const &[key, value] : map.boolFields)
        if (key == "interactable")
            interactable = value;
}