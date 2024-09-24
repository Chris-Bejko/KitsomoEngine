#pragma once

#include "Player.h"
class FloorSquare : public Component
{
public:
	FloorSquare() = default;

	virtual ~FloorSquare() = default;
	bool Init() override final
	{
		AssetManager::get().loadTexture("square", "square.png");
		sprite = &entity->AddComponent<Sprite>("square");
		entity->transform->scale = Vector2F(0.2, 0.2);
		Serialize();
		return true;
	}

	void Config(Vector2F position, sf::Color color)
	{
		this->color = color;
		std::cout << color.a << std::endl;
		entity->transform->position = position;
		entity->AddComponent<BoxCollider>("floor", sf::FloatRect(0, 0, 125, 125), true);
		sprite->SetColor(color);
	}

	void Serialize()
	{
		variables.push_back({ "color.r",&color.r, int_Type });
		variables.push_back({ "color.g",&color.g, int_Type });
		variables.push_back({ "color.b",&color.b, int_Type });
		variables.push_back({ "color.a",&color.a, int_Type });
	}
	void InitSerializedFields(ReadableSerializableVariableMap map)
	{
		for(auto const& [key, value] : map.intFields)
		{
			if(key == "color.r")
			{
				color.r = value;
			}
			if (key == "color.g")
			{
				color.g = value;
			}
			if (key == "color.b")
			{
				color.b = value;
			}
			if (key == "color.a")
			{
				color.a = value;
			}
		}
	}

	std::vector<SerializableVariable>* GetSerializedFields() override final
	{
		return &variables;
	}

	sf::Color GetColor()
	{
		return color;
	}

	void OnTriggerStay(BoxCollider& other) override final
	{
		std::cout << "On Trigger Stay" << std::endl;
	}
private:
	std::vector<SerializableVariable> variables;
	sf::Color color;
	Sprite* sprite;
};