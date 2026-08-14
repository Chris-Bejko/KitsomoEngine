#include "Sprite.h"

#include "BoxCollider.h"
#include "CommandHistory.h"
#include "Commands/MoveEntityCommand.h"
#include "ComponentRegistry.h"
#include "Engine.h"
#include "GizmoSystem.h"
#include "ImguiHandler.h"
#include "Logger.h"

DECLARE_COMPONENT_RULES(Sprite, false)
REGISTER_COMPONENT(Sprite)
REGISTER_SERIALIZABLE_COMPONENT(Sprite)

Sprite::Sprite(
    std::string texturePath,
    int renderOrder,
    Color color)
{
    _texture.SetPath(texturePath);

    ColorID = color.SerializeColor();

    sprite.setColor(color.GetColorEnum());

    SetRenderOrder(renderOrder);
}

bool Sprite::Init()
{
    Field("Texture", _texture);
    Field("ColorID", ColorID);
    Field("renderOrder", renderOrder);

    RefreshTexture();
    RefreshColor();

    return true;
}

void Sprite::RefreshTexture()
{
    const std::string& path = _texture.GetPath();

    if (path.empty())
    {
        texture = nullptr;
        width = 0;
        height = 0;
        lastTexturePath.clear();
        return;
    }

    if (path == lastTexturePath && texture != nullptr)
        return;

    AssetManager::get().loadTexture(path);

    texture = AssetManager::get().getTexture(path);

    sprite.setTexture(*texture);

    width = static_cast<int>(texture->getSize().x);
    height = static_cast<int>(texture->getSize().y);

    sprite.setOrigin(
        static_cast<sf::Vector2f>(texture->getSize()) / 2.f);

    lastTexturePath = path;
}

void Sprite::RefreshColor()
{
    if (ColorID == lastColorID)
        return;

    Color color;
    color.SetColor(ColorID);

    sprite.setColor(color.GetColorEnum());

    lastColorID = ColorID;
}

void Sprite::OnFieldChanged(
    const std::string &fieldName)
{
    if (fieldName == "Texture")
    {
        RefreshTexture();
    }
    else if (fieldName == "ColorID")
    {
        RefreshColor();
    }
}

void Sprite::draw()
{
    Engine::get().Draw(this);
}

void Sprite::update(float dt)
{
    RefreshTexture();
    RefreshColor();

    if (entity == nullptr)
        return;

    if (texture != nullptr)
        sprite.setTexture(*texture);

    auto worldPos = entity->transform->GetWorldPosition();
    auto worldRot = entity->transform->GetWorldRotation();
    auto worldScale = entity->transform->GetWorldScale();

    sprite.setPosition(worldPos.x, worldPos.y);
    sprite.setRotation(worldRot);
    sprite.setScale(worldScale.x, worldScale.y);
}

void Sprite::updateEngine(float dt)
{
    update(dt);
}

int Sprite::GetHeight()
{
    return height;
}

int Sprite::GetWidth()
{
    return width;
}

sf::Vector2f Sprite::GetScale()
{
    return sprite.getScale();
}

sf::FloatRect Sprite::GetGlobalBounds()
{
    return sprite.getGlobalBounds();
}

sf::FloatRect Sprite::TranslateHitbox(
    sf::FloatRect &hitbox)
{
    return sprite.getTransform().transformRect(hitbox);
}

sf::Vector2f Sprite::GetPosition()
{
    return sprite.getPosition();
}

sf::Vector2f Sprite::GetOrigin()
{
    return texture ? static_cast<sf::Vector2f>(texture->getSize()) / 2.f : sf::Vector2f();
}

sf::Sprite Sprite::GetSprite()
{
    return sprite;
}

void Sprite::SetColor(const sf::Color &color)
{
    sprite.setColor(color);
}

void Sprite::SetColor(Color color)
{
    ColorID = color.SerializeColor();

    if (entity != nullptr && entity->GetName().find("Bullet") != std::string::npos)
    {
        LOG_DEBUG("Setting bullet color to: ", ColorID);
    }

    sprite.setColor(color.GetColorEnum());

    lastColorID = ColorID;
}

void Sprite::SetOrigin(
    const Vector2F &origin)
{
    sprite.setOrigin(
        sf::Vector2f(origin.x, origin.y));
}

sf::Vector2f Sprite::GetRotation()
{
    return sf::Vector2f();
}

bool Sprite::isMouseOver(
    const sf::Sprite &sprite,
    int mouseX,
    int mouseY)
{
    return sprite.getGlobalBounds().contains(static_cast<float>(mouseX), static_cast<float>(mouseY));
}

int Sprite::RenderOrder()
{
    return renderOrder;
}

void Sprite::SetRenderOrder(int i)
{
    renderOrder = i;
}