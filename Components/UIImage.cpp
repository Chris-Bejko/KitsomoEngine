#include "UIImage.h"
#include "Canvas.h"
#include "../Engine.h"
#include "../AssetManager.h"
#include "../UI/UIRect.h"

UIImage::UIImage(const std::string &texId)
{
    textureId = texId;
}

bool UIImage::Init()
{
    Serialize();
    if (!entity->HasComponent<UIRect>())
        entity->AddComponent<UIRect>();
    if (!textureId.empty())
        UpdateSprite();
    return true;
}

void UIImage::UpdateSprite()
{
    if (textureId.empty())
        return;
    AssetManager::get().loadTexture(textureId, textureId + ".png");
    uiTexture = AssetManager::get().getTexture(textureId);
    uiSprite.setTexture(uiTexture);
}

void UIImage::draw()
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

    // Scale sprite to fit size
    if (uiTexture.getSize().x > 0 && uiTexture.getSize().y > 0)
    {
        sf::Vector2f size = entity->GetComponent<UIRect>().sizeDelta;
        uiSprite.setScale(
            size.x / uiTexture.getSize().x,
            size.y / uiTexture.getSize().y);
    }
    uiSprite.setPosition(screenPos);
    Engine::get().GetWindow().draw(uiSprite);

    if (screenSpace)
        Engine::get().GetWindow().setView(prevView);
}

void UIImage::update(float dt) {}
void UIImage::updateEngine(float dt) { update(dt); }

void UIImage::SetTexture(const std::string &texId)
{
    textureId = texId;
    UpdateSprite();
}

void UIImage::SetColor(sf::Color color) { uiSprite.setColor(color); }

void UIImage::SetAlpha(float a)
{
    alpha = a;
    sf::Color c = uiSprite.getColor();
    c.a = (sf::Uint8)(a * 255);
    uiSprite.setColor(c);
}

void UIImage::Serialize()
{
    serializables.push_back({"textureId", &textureId, char_Type});
    serializables.push_back({"color", &colorString, char_Type});
    serializables.push_back({"alpha", &alpha, float_Type});
}

void UIImage::InitSerializedFields(ReadableSerializableVariableMap map)
{
    for (auto const &[key, value] : map.stringFields)
    {
        if (key == "textureId")
        {
            textureId = value;
            UpdateSprite();
        }
        if (key == "color")
            colorString = value;
    }
    for (auto const &[key, value] : map.floatFields)
        if (key == "alpha")
            SetAlpha(value);
}