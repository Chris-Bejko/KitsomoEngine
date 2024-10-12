#include "FloorSquare.h"
#include "../Entity.h"

bool FloorSquare::Init()
{
	AssetManager::get().loadTexture("square", "square.png");
	sprite = &entity->AddComponent<Sprite>("square");
	entity->GetComponent<Sprite>().SetRenderOrder(1);
	entity->transform->scale = Vector2F(0.2, 0.2);
	Serialize();
	return true;
}

void FloorSquare::Config(Vector2F position, Color color)
{
	this->color = color;
	entity->transform->position = position;
	entity->AddComponent<BoxCollider>("floor", sf::FloatRect(0, 0, 125, 125), true);
	sprite->SetColor(color.GetColor());
}

void FloorSquare::Serialize()
{
	variables.push_back({ "color",&colorString, char_Type });
}

void FloorSquare::InitSerializedFields(ReadableSerializableVariableMap map)
{
	for (auto const& [key, value] : map.stringFields)
	{
		if (key == "color")
		{
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
	std::cout << "On Trigger Stay" << std::endl;
}
