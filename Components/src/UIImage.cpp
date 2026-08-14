#include "UIImage.h"
#include "Canvas.h"
#include "Engine.h"
#include "AssetManager.h"
#include "UIRect.h"
#include "ComponentRegistry.h"

DECLARE_COMPONENT_RULES(UIImage, false)
REGISTER_COMPONENT(UIImage)
REGISTER_SERIALIZABLE_COMPONENT(UIImage)

UIImage::UIImage(const std::string& texturePath)
{
    _texture.SetPath(texturePath);
}

bool UIImage::Init()
{
    Field("Texture", _texture);
    Field("color", colorString);
    Field("alpha", alpha);

    if (!entity->HasComponent<UIRect>())
        entity->AddComponent<UIRect>();

    UpdateSprite();

    return true;
}

void UIImage::UpdateSprite()
{
    const std::string& path = _texture.GetPath();

    if (path.empty())
    {
        uiTexture = nullptr;
        return;
    }

    AssetManager::get().loadTexture(path);
    uiTexture = AssetManager::get().getTexture(path);

    if (uiTexture == nullptr)
        return;

    uiSprite.setTexture(*uiTexture, true);
}

void UIImage::draw()
{
    Canvas* canvas = entity->GetComponent<UIRect>().GetCanvas();

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

    if (uiTexture && uiTexture->getSize().x > 0 && uiTexture->getSize().y > 0)
    {
        Vector2F size = entity->GetComponent<UIRect>().sizeDelta;

        uiSprite.setScale(
            size.x / uiTexture->getSize().x,
            size.y / uiTexture->getSize().y);
    }

    uiSprite.setPosition(screenPos.x, screenPos.y);

    Engine::get().GetWindow().draw(uiSprite);

    if (screenSpace)
        Engine::get().GetWindow().setView(prevView);
}

void UIImage::update(float dt)
{
    UpdateSprite();
}

void UIImage::updateEngine(float dt)
{
    update(dt);
}

void UIImage::SetTexture(const std::string& texturePath)
{
    _texture.SetPath(texturePath);
    UpdateSprite();
}

void UIImage::SetColor(sf::Color color)
{
    uiSprite.setColor(color);
}

void UIImage::SetAlpha(float a)
{
    alpha = a;

    sf::Color c = uiSprite.getColor();
    c.a = static_cast<sf::Uint8>(a * 255);
    uiSprite.setColor(c);
}