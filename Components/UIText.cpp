#include "UIText.h"
#include "Canvas.h"
#include "../Engine.h"
#include "../AssetManager.h"
#include "../Color.h"
#include "../UI/UIRect.h"
UIText::UIText(const std::string &text, const std::string &fontId, unsigned int fontSize)
    : text(text), fontId(fontId), fontSize(fontSize)
{
}

bool UIText::Init()
{
    if (!entity->HasComponent<UIRect>())
        entity->AddComponent<UIRect>();
    Serialize();
    if (!fontId.empty())
        UpdateTextProperties();
    return true;
}

void UIText::UpdateTextProperties()
{
    if (fontId.empty())
        return;
    auto *font = AssetManager::get().getFont(fontId);
    if (!font)
        return;

    sfText.setFont(*font);
    sfText.setString(text);
    sfText.setCharacterSize(fontSize);

    Color c;
    c.SetColor(colorString);
    sfText.setFillColor(c.GetColorEnum());
}

void UIText::ApplyAlignment(sf::Vector2f screenPos)
{
    sf::FloatRect bounds = sfText.getLocalBounds();

    sf::Vector2f size = entity->GetComponent<UIRect>().sizeDelta;
    sf::Vector2f pivot = entity->GetComponent<UIRect>().pivot;
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

    sfText.setPosition(screenPos.x + size.x * pivot.x, screenPos.y + size.y * pivot.y);
}

void UIText::draw()
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

    UpdateTextProperties();
    sf::Vector2f screenPos = entity->GetComponent<UIRect>().GetScreenPosition();
    ApplyAlignment(screenPos);
    Engine::get().GetWindow().draw(sfText);

    if (screenSpace)
        Engine::get().GetWindow().setView(prevView);
}

void UIText::update(float dt) { /* handled by draw */ }
void UIText::updateEngine(float dt) { update(dt); }

void UIText::SetText(const std::string &t) { text = t; }
void UIText::SetFontSize(unsigned int s) { fontSize = s; }
void UIText::SetColor(sf::Color c) { sfText.setFillColor(c); }
void UIText::SetAlignment(TextAlignment a) { alignment = a; }

void UIText::Serialize()
{
    serializables.push_back({"text", &text, char_Type});
    serializables.push_back({"fontId", &fontId, char_Type});
    serializables.push_back({"fontSize", (int *)&fontSize, int_Type});
    serializables.push_back({"color", &colorString, char_Type});
    serializables.push_back({"alignment", &alignmentString, char_Type});
}

void UIText::InitSerializedFields(ReadableSerializableVariableMap map)
{
    for (auto const &[key, value] : map.stringFields)
    {
        if (key == "text")
            text = value;
        if (key == "fontId")
            fontId = value;
        if (key == "color")
            colorString = value;
        if (key == "alignment")
        {
            alignmentString = value;
            if (value == "Left")
                alignment = TextAlignment::Left;
            else if (value == "Right")
                alignment = TextAlignment::Right;
            else
                alignment = TextAlignment::Center;
        }
    }
    for (auto const &[key, value] : map.intFields)
        if (key == "fontSize")
            fontSize = value;
}