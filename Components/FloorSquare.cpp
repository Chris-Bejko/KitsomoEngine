#include "FloorSquare.h"
#include "../Entity.h"
#include "../Logger.h"
bool FloorSquare::Init()
{
	AssetManager::get().loadTexture("square", "square.png");
	Color color;
	color.SetColor(colorString);
	sprite = &entity->AddComponent<Sprite>("square", 1, color);
	entity->transform->scale = Vector2F(0.2, 0.2);
	Serialize();
	return true;
}

void FloorSquare::Config(Vector2F position, Color color)
{
	this->color = color;
	entity->transform->position = position;
	entity->AddComponent<BoxCollider>("floor", sf::FloatRect(0, 0, 125, 125), true);
	sprite->SetColor(color);
}

void FloorSquare::Serialize()
{
	variables.push_back({ "colorString",&colorString, char_Type });
}

void FloorSquare::InitSerializedFields(ReadableSerializableVariableMap map)
{
	for (auto const& [key, value] : map.stringFields)
	{
		if (key == "colorString")
		{
			colorString = value;
			color.SetColor(value);
		}
	}
}

void FloorSquare::update(float dt)
{
	color.SetColor(colorString);
}

void FloorSquare::updateEngine(float dt)
{
	color.SetColor(colorString);
}

std::vector<SerializableVariable>* FloorSquare::GetSerializedFields()
{
	return &variables;
}

Color FloorSquare::GetColor()
{
	return color;
}

void FloorSquare::OnTriggerStay(BoxCollider& other)
{
	LOG_DEBUG("ON TRIGGER STAY");
}
