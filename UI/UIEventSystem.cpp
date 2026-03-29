#include "UIEventSystem.h"
#include "../Components/UIButton.h"
#include "../Components/Canvas.h"
#include "../Engine.h"
#include "../UI/UIRect.h"

void UIEventSystem::Register(UIButton *button)
{
    registeredGUIDs.push_back(button->GetGUID());
}

void UIEventSystem::Unregister(UIButton *button)
{
    auto &guid = button->GetGUID();
    registeredGUIDs.erase(
        std::remove(registeredGUIDs.begin(), registeredGUIDs.end(), guid),
        registeredGUIDs.end());
}

void UIEventSystem::Clear()
{
    registeredGUIDs.clear();
    hoveredButtons.clear();
}

void UIEventSystem::Update()
{
    if (Engine::get().isEngine)
    return; // only in play mode
    auto mousePixel = sf::Mouse::getPosition(Engine::get().GetWindow());
    sf::View defaultView = Engine::get().GetWindow().getDefaultView();
    sf::Vector2f mouse = Engine::get().GetWindow().mapPixelToCoords(mousePixel, defaultView);
    
    bool mousePressed = sf::Mouse::isButtonPressed(sf::Mouse::Left);
    for (auto &guid : registeredGUIDs)
    {
        // Find button by component GUID
        UIButton *button = nullptr;
        for (auto &e : Engine::get().GetManager()->GetEntities())
        {
            if (e->IsPendingDestroy())
                continue;
            if (!e->HasComponent<UIButton>())
                continue;
            auto &btn = e->GetComponent<UIButton>();
            if (btn.GetGUID() == guid)
            {
                button = &btn;
                break;
            }
        }

        if (!button)
            continue;
        if (!button->entity)
            continue;

        if(!button->entity->IsActiveInHierarchy())
            continue;
        Canvas *canvas = button->entity->GetComponent<UIRect>().GetCanvas();
        if (!canvas)
            continue;

        sf::Vector2f mousePos = mouse;

        // For world space canvas, convert mouse to world coords
        if (canvas->GetRenderMode() == CanvasRenderMode::WorldSpace)
        {
            mousePos = Engine::get().GetWindow().mapPixelToCoords(
                sf::Mouse::getPosition(Engine::get().GetWindow()));
        }

        sf::FloatRect rect = button->entity->GetComponent<UIRect>().GetScreenRect();
        bool isOver = rect.contains(mousePos);

        if (isOver)
        {
            if (hoveredButtons.find(button) == hoveredButtons.end())
            {
                hoveredButtons.insert(button);
                button->isHovered = true;
                button->FireOnHover();
            }

            if (mousePressed)
            {
                button->isPressed = true;
                button->FireOnClick();
            }
            else
            {
                button->isPressed = false;
            }
        }
        else
        {
            if (hoveredButtons.find(button) != hoveredButtons.end())
            {
                hoveredButtons.erase(button);
                button->isHovered = false;
                button->isPressed = false;
                button->FireOnHoverExit();
            }
        }
    }
}