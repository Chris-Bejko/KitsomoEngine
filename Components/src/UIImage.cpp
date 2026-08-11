#include "UIImage.h"
#include "Canvas.h"
#include "Engine.h"
#include "AssetManager.h"
#include "UIRect.h"
#include "ComponentRegistry.h"

DECLARE_COMPONENT_RULES(UIImage, false)
REGISTER_COMPONENT(UIImage)
REGISTER_SERIALIZABLE_COMPONENT(UIImage)

UIImage::UIImage(const std::string &texId)
{
    textureId = texId;
}

bool UIImage::Init()
{
    Field("textureId", textureId);
    Field("color", colorString);
    Field("alpha", alpha);
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

    Vector2F screenPos = entity->GetComponent<UIRect>().GetScreenPosition();

    // Scale sprite to fit size
    if (uiTexture.getSize().x > 0 && uiTexture.getSize().y > 0)
    {
        Vector2F size = entity->GetComponent<UIRect>().sizeDelta;
        uiSprite.setScale(
            size.x / uiTexture.getSize().x,
            size.y / uiTexture.getSize().y);
    }
    sf::Vector2f screenPosSf = sf::Vector2f(screenPos.x, screenPos.y);
    uiSprite.setPosition(screenPosSf);
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