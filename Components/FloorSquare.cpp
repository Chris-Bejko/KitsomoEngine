#include "FloorSquare.h"
#include "../Entity.h"
#include "../Logger.h"
#include "Bullet.h"
#include "BoxCollider.h"
bool FloorSquare::Init()
{
	AssetManager::get().loadTexture("square", "square.png");
	Color color;
	color.SetColor(colorString);
	entity->transform->scale = Vector2F(0.2, 0.2);
	Serialize();
	return true;
}

void FloorSquare::Config(Vector2F position, Color color)
{
	this->color = color;
	entity->transform->position = position;
	if(entity->HasComponent<Sprite>())
	{
		entity->GetComponent<Sprite>().SetColor(color);
	}
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
	if(entity->HasComponent<Sprite>())
	{
		entity->GetComponent<Sprite>().SetColor(color);
	}
}

void FloorSquare::updateEngine(float dt)
{
	color.SetColor(colorString);
}

std::vector<SerializableVariable>* FloorSquare::GetSerializedFields()
{
	return &variables;
}

Color FloorSquare::GetColorEnum()
{
	return color;
}

void FloorSquare::OnTriggerStay(Collider& other)
{
	LOG_DEBUG("ON TRIGGER STAY");
}

void FloorSquare::OnTriggerEnter(Collider& other)
{
	if (other.GetCollisionTag() == "bullet")
	{
		LOG_INFO("Bullet is on the floor, changing color to match the floor's color");
		other.entity->GetComponent<Bullet>().SetColor(color);
	}
}