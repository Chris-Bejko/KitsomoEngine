#pragma once
#include <functional>
#include <vector>
#include <SFML/Graphics.hpp>
#include "../SerializableScript.h"

class UIButton : public SerializableScript
{
public:
    UIButton();
    virtual ~UIButton();

    bool Init() override;
    void draw() override;
    void update(float dt) override;
    void updateEngine(float dt) override;
    void Awake() override;

    // Event subscription
    void AddOnClick(std::function<void()> callback)     { onClickListeners.push_back(callback); }
    void AddOnHover(std::function<void()> callback)     { onHoverListeners.push_back(callback); }
    void AddOnHoverExit(std::function<void()> callback) { onHoverExitListeners.push_back(callback); }

    // Called by UIEventSystem
    void FireOnClick()     { for (auto& f : onClickListeners)     f(); }
    void FireOnHover()     { for (auto& f : onHoverListeners)     f(); }
    void FireOnHoverExit() { for (auto& f : onHoverExitListeners) f(); }

    bool IsInteractable() { return interactable; }

    // Visual states
    sf::Color normalColor     = sf::Color(255, 255, 255, 200);
    sf::Color hoverColor      = sf::Color(200, 200, 255, 230);
    sf::Color pressedColor    = sf::Color(150, 150, 200, 255);
    sf::Color disabledColor   = sf::Color(128, 128, 128, 128);
    bool interactable = true;
    bool isHovered = false;
    bool isPressed = false;

    void DrawEditorButton() override;
private:
    std::vector<std::function<void()>> onClickListeners;
    std::vector<std::function<void()>> onHoverListeners;
    std::vector<std::function<void()>> onHoverExitListeners;

    sf::RectangleShape background;

    void UpdateVisualState();
};