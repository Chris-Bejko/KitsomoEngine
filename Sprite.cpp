#include "Components/Sprite.h"
#include "../Engine.h"
#include "../imguiHandler.h"

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
			AssetManager::get().loadTexture(textureID, textureID + ".png");
			texture = AssetManager::get().getTexture(textureID);
			std::cout << textureID << std::endl;
			sprite.setOrigin((sf::Vector2f)texture.getSize() / 2.f);
			sprite.setTexture(texture);
		}
	}
}

void Sprite::draw()
{
	Engine::get().GetWindow().draw(sprite);
}

void Sprite::update(float dt)
{
	sprite.setTexture(texture);
	sprite.setPosition(entity->GetComponent<Transform>().position.x, entity->GetComponent<Transform>().position.y);
	sprite.setRotation(entity->GetComponent<Transform>().rotation);
	sprite.setScale(sf::Vector2f(entity->GetComponent<Transform>().scale.x, entity->GetComponent<Transform>().scale.y));

	if (dragging)
	{
		std::cout << "Should drag to entity: " << entity->GetName() << std::endl;
		entity->GetComponent<Transform>().SetPosition(mousePos.x - mouseRectOffset.x, mousePos.y - mouseRectOffset.y);
	}
}

int Sprite::GetHeight()
{
	return height;
}

int Sprite::GetWidth()
{
	return width;
}

void Sprite::updateEngine(float dt)
{
	update(dt);

	if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
	{
		mousePos = Engine::get().GetWindow().mapPixelToCoords(sf::Mouse::getPosition(Engine::get().GetWindow()));

		if (isMouseOver(sprite, mousePos.x, mousePos.y) && (!Engine::get().DraggingEntity() || Engine::get().GetDraggedEntity() == entity->GetName()))
		{
			Engine::get().TriggerDragging(entity->GetName());
			ImguiHandler::get().ClearInspector();
			entity->displayComponents = true;
			dragging = true;
		}
	}
	else
	{
		Engine::get().TriggerDragging("");
		dragging = false;
	}
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

bool Sprite::isMouseOver(const sf::Sprite& sprite, int mouseX, int mouseY)
{
	return sprite.getGlobalBounds().contains(mouseX, mouseY);
}
