#include "UIEventSystem.h"
#include "../Components/UIButton.h"
#include "../Components/Canvas.h"
#include "../Engine.h"
#include "../UI/UIRect.h"

void UIEventSystem::Register(UIButton *button)
{
    buttons.push_back(button);
}

void UIEventSystem::Unregister(UIButton *button)
{
    buttons.erase(std::remove(buttons.begin(), buttons.end(), button), buttons.end());
    hoveredButtons.erase(button);
}

void UIEventSystem::Clear()
{
    buttons.clear();
    hoveredButtons.clear();
}

void UIEventSystem::Update()
{
    if (Engine::get().isEngine)
        return; // only in play mode

    sf::Vector2i pixelPos = sf::Mouse::getPosition(Engine::get().GetWindow());
    sf::View defaultView = Engine::get().GetWindow().getDefaultView();
    sf::Vector2f mouseScreen = Engine::get().GetWindow().mapPixelToCoords(pixelPos, defaultView);
    // Get mouse in screen space
    bool mousePressed = sf::Mouse::isButtonPressed(sf::Mouse::Left);

    for (auto *button : buttons)
    {
        if (!button || !button->IsInteractable())
            continue;

        Canvas *canvas = button->entity->GetComponent<UIRect>().GetCanvas();
        if (!canvas)
            continue;

        sf::Vector2f mousePos = mouseScreen;

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