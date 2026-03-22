#include "UIText.h"
#include "Canvas.h"
#include "../Engine.h"
#include "../AssetManager.h"
#include "../Color.h"
#include "../UI/UIRect.h"

UIText::UIText(const std::string& text, const std::string& fontId, unsigned int fontSize)
    : text(text), fontId(fontId), fontSize(fontSize)
{
}

bool UIText::Init()
{
    if (!entity->HasComponent<UIRect>())
        entity->AddComponent<UIRect>();
    Field("text",      text);
    Field("fontId",    fontId);
    Field("fontSize",  fontSize);
    Field("color",     colorString);
    Field("alignment", alignmentString);
    if (!fontId.empty())
        UpdateTextProperties();
    return true;
}

void UIText::UpdateTextProperties()
{
    if (fontId.empty()) return;
    auto* font = AssetManager::get().getFont(fontId);
    if (!font) return;

    sfText.setFont(*font);
    sfText.setString(text);
    sfText.setCharacterSize(fontSize);

    Color c;
    c.SetColor(colorString);
    sfText.setFillColor(c.GetColorEnum());
}

void UIText::ApplyAlignment(Vector2F screenPos)
{
    sf::FloatRect bounds = sfText.getLocalBounds();

    auto& uiRect = entity->GetComponent<UIRect>();
    Vector2F size  = uiRect.sizeDelta;
    Vector2F pivot = uiRect.pivot;

    switch (alignment)
    {
    case TextAlignment::Left:
        sfText.setOrigin(0, bounds.height * pivot.y);
        break;
    case TextAlignment::Center:
        sfText.setOrigin(bounds.width * 0.5f, bounds.height * pivot.y);
        break;
    case TextAlignment::Right:
        sfText.setOrigin(bounds.width, bounds.height * pivot.y);
        break;
    }

    sfText.setPosition(screenPos.x + size.x * pivot.x,
                       screenPos.y + size.y * pivot.y);
}

void UIText::draw()
{
    Canvas* canvas = entity->GetComponent<UIRect>().GetCanvas();
    if (!canvas) return;

    sf::View prevView;
    bool screenSpace = canvas->GetRenderMode() == CanvasRenderMode::ScreenSpace;

    if (screenSpace)
    {
        prevView = Engine::get().GetWindow().getView();
        Engine::get().GetWindow().setView(Engine::get().GetWindow().getDefaultView());
    }

    UpdateTextProperties();
    Vector2F screenPos = entity->GetComponent<UIRect>().GetScreenPosition();
    ApplyAlignment(screenPos);
    Engine::get().GetWindow().draw(sfText);

    if (screenSpace)
        Engine::get().GetWindow().setView(prevView);
}

void UIText::update(float dt) {}
void UIText::updateEngine(float dt) { update(dt); }

void UIText::SetText(const std::string& t)  { text = t; }
void UIText::SetFontSize(unsigned int s)     { fontSize = s; }
void UIText::SetColor(sf::Color c)           { sfText.setFillColor(c); }
void UIText::SetAlignment(TextAlignment a)   { alignment = a; }