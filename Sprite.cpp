#include "Components/Sprite.h"
#include "../Engine.h"

Sprite::Sprite(std::string textureId)
{
	textureID = textureId;
}

bool Sprite::Init()
{
	Serialize();
	texture = AssetManager::get().getTexture(textureID);
	sprite.setTexture(texture);
	sprite.setOrigin((sf::Vector2f)texture.getSize() / 2.f);
	return true;
}

std::vector<SerializableVariable>* Sprite::GetSerializedFields()
{
	return &variables;
}

void Sprite::Serialize()
{
	variables.push_back({ "textureID", &textureID, char_Type });
}

void Sprite::InitSerializedFields(ReadableSerializableVariableMap map)
{
	for (auto const& [key, value] : map.stringFields)
	{
		if (key == "textureID")
		{
			textureID = value;
		}
	}
}

void Sprite::draw()
{
	Engine::get().GetWindow().draw(sprite);
}

void Sprite::update(float dt)
{
	sprite.setPosition(entity->GetComponent<Transform>().position.x, entity->GetComponent<Transform>().position.y);
	sprite.setRotation(entity->GetComponent<Transform>().rotation);
	sprite.setScale(sf::Vector2f(entity->GetComponent<Transform>().scale.x, entity->GetComponent<Transform>().scale.y));
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

sf::FloatRect Sprite::TranslateHitbox(sf::FloatRect& hitbox)
{
	return sprite.getTransform().transformRect(hitbox);
}

sf::Vector2f Sprite::GetPosition()
{
	return sprite.getPosition();
}

sf::Vector2f Sprite::GetOrigin()
{
	return (sf::Vector2f)texture.getSize() / 2.0f;
}

sf::Sprite Sprite::GetSprite()
{
	return sprite;
}

void Sprite::SetColor(const sf::Color& color)
{
	sprite.setColor(color);
}

void Sprite::SetOrigin(const Vector2F& origin)
{
	sprite.setOrigin(sf::Vector2f(origin.x, origin.y));
}

sf::Vector2f Sprite::GetRotation()
{
	return sf::Vector2f();
}
