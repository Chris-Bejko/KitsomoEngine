#include "Components/Sprite.h"
#include "../Engine.h"
#include "../imguiHandler.h"
#include "Logger.h"
#include "Components/BoxCollider.h"
#include "../Commands/MoveEntityCommand.h"
#include "../Commands/CommandHistory.h"

Sprite::Sprite(std::string textureId, int renderOrder, Color color)
{
	textureID = textureId;
	ColorID = color.SerializeColor();
	sprite.setColor(color.GetColorEnum());
	SetRenderOrder(renderOrder);
}

bool Sprite::Init()
{
	Serialize();
	if (!textureID.empty())
	{
		// Try to load texture if not already in AssetManager
		AssetManager::get().loadTexture(textureID, textureID + ".png");
		texture = AssetManager::get().getTexture(textureID);
		sprite.setTexture(texture);
		sprite.setOrigin((sf::Vector2f)texture.getSize() / 2.f);
		LOG_DEBUG("Texture '", textureID, "' size: ", texture.getSize().x, "x", texture.getSize().y);
	}
	return true;
}

std::vector<SerializableVariable> *Sprite::GetSerializedFields()
{
	return &variables;
}

void Sprite::Serialize()
{
	variables.push_back({"textureID", &textureID, char_Type});
	variables.push_back({"ColorID", &ColorID, char_Type});
	variables.push_back({"renderOrder", &renderOrder, int_Type});
}

void Sprite::InitSerializedFields(ReadableSerializableVariableMap map)
{
	for (auto const &[key, value] : map.stringFields)
	{
		LOG_DEBUG(key, value);
		if (key == "textureID")
		{
			textureID = value;
			AssetManager::get().loadTexture(textureID, textureID + ".png");
			texture = AssetManager::get().getTexture(textureID);
			sprite.setOrigin((sf::Vector2f)texture.getSize() / 2.f);
			sprite.setTexture(texture);
		}

		if (key == "ColorID")
		{
			ColorID = value;
			Color color;
			color.SetColor(value);
			SetColor(color);
		}
	}

	for (auto const &[key, value] : map.intFields)
	{
		if (key == "renderOrder")
		{
			renderOrder = value;
		}
	}
}

void Sprite::draw()
{
	// LOG_DEBUG("Drawing sprite at: ", sprite.getPosition().x, ", ", sprite.getPosition().y, " texture: ", textureID, " color: ", ColorID);
	Engine::get().GetWindow().draw(sprite);
}

void Sprite::update(float dt)
{
	if (textureID != lastTextureID)
	{
		AssetManager::get().loadTexture(textureID, textureID + ".png");
		texture = AssetManager::get().getTexture(textureID);
		sprite.setTexture(texture);
		sprite.setOrigin((sf::Vector2f)texture.getSize() / 2.f);
		lastTextureID = textureID;
	}
	if (ColorID != lastColorID)
	{
		Color color;
		color.SetColor(ColorID);
		sprite.setColor(color.GetColorEnum());
		lastColorID = ColorID;
	}
	sprite.setTexture(texture);
	sprite.setPosition(entity->GetComponent<Transform>().position.x, entity->GetComponent<Transform>().position.y);
	sprite.setRotation(entity->GetComponent<Transform>().rotation);
	sprite.setScale(sf::Vector2f(entity->GetComponent<Transform>().scale.x, entity->GetComponent<Transform>().scale.y));

	// Track drag start
	if (dragging && !wasDragging)
	{
		dragStartPosition = entity->GetComponent<Transform>().position;
	}

	if (dragging)
	{
		entity->GetComponent<Transform>().SetPosition(mousePos.x - mouseRectOffset.x, mousePos.y - mouseRectOffset.y);
	}

	// Track drag end and create command
	if (!dragging && wasDragging)
	{
		Vector2F currentPos = entity->GetComponent<Transform>().position;
		// Only create a command if the position actually changed
		if (currentPos.x != dragStartPosition.x || currentPos.y != dragStartPosition.y)
		{
			auto moveCmd = std::make_unique<MoveEntityCommand>(entity, dragStartPosition, currentPos);
			CommandHistory::get().Execute(std::move(moveCmd));
		}
	}

	wasDragging = dragging;
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

	if (entity->HasComponent<BoxCollider>() && entity->GetComponent<BoxCollider>().editMode)
		return;

	if (forceDrag)
    {
        mousePos = Engine::get().GetWindow().mapPixelToCoords(
            sf::Mouse::getPosition(Engine::get().GetWindow()));
        Engine::get().TriggerDragging(entity->GetName());
        entity->displayComponents = true;
        Engine::get().GetManager()->SetSelectedEntity(entity);
        dragging = true;
        forceDrag = false; // only trigger once
    }
	
	if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
	{
		mousePos = Engine::get().GetWindow().mapPixelToCoords(sf::Mouse::getPosition(Engine::get().GetWindow()));

		if (isMouseOver(sprite, mousePos.x, mousePos.y) && (!Engine::get().DraggingEntity() || Engine::get().GetDraggedEntity() == entity->GetName()))
		{
			if (!dragging && !pendingDrag)
			{
				float currentTime = ImGui::GetTime();
				if (lastClickTime > 0.f && (currentTime - lastClickTime) < 0.3f)
				{
					Engine::get().FocusOnEntity(entity);
					lastClickTime = 0.f;
				}
				else
				{
					lastClickTime = currentTime;
				}
				pendingDrag = true;
				dragTimer = 0.f;
			}

			if (pendingDrag)
			{
				dragTimer += dt;
				if (dragTimer >= dragDelay)
				{
					Engine::get().TriggerDragging(entity->GetName());
					ImguiHandler::get().ClearInspector();
					entity->displayComponents = true;
					Engine::get().GetManager()->SetSelectedEntity(entity);
					dragging = true;
					pendingDrag = false;
				}
			}
		}
	}
	else
	{
		Engine::get().TriggerDragging("");
		dragging = false;
		pendingDrag = false;
		dragTimer = 0.f;
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

sf::FloatRect Sprite::TranslateHitbox(sf::FloatRect &hitbox)
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

void Sprite::SetColor(const sf::Color &color)
{
	sprite.setColor(color);
}

void Sprite::SetColor(Color color)
{
	ColorID = color.SerializeColor();
	if (entity->GetName().find("Bullet") != std::string::npos)
		LOG_DEBUG("Setting bullet color to: ", ColorID);
	sprite.setColor(color.GetColorEnum());
}

void Sprite::SetOrigin(const Vector2F &origin)
{
	sprite.setOrigin(sf::Vector2f(origin.x, origin.y));
}

sf::Vector2f Sprite::GetRotation()
{
	return sf::Vector2f();
}

bool Sprite::isMouseOver(const sf::Sprite &sprite, int mouseX, int mouseY)
{
	return sprite.getGlobalBounds().contains(mouseX, mouseY);
}

int Sprite::RenderOrder()
{
	return renderOrder;
}

void Sprite::SetRenderOrder(int i)
{
	renderOrder = i;
}
