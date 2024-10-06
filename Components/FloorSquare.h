#pragma once

#include "Player.h"
#include "../Color.h"
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

	void Config(Vector2F position, Color color)
	{
		this->color = color;
		entity->transform->position = position;
		entity->AddComponent<BoxCollider>("floor", sf::FloatRect(0, 0, 125, 125), true);
		sprite->SetColor(color.GetColor());
	}

	void Serialize()
	{
		variables.push_back({ "color",&colorString, char_Type });
	}
	void InitSerializedFields(ReadableSerializableVariableMap map)
	{
		for (auto const& [key, value] : map.stringFields)
		{
			if (key == "color")
			{
				color.SetColor(value);
			}
		}
	}

	void update(float dt) override final
	{
		color.SetColor(colorString);
	}

	void updateEngine(float dt) override final
	{
		color.SetColor(colorString);
	}

	std::vector<SerializableVariable>* GetSerializedFields() override final
	{
		return &variables;
	}

	Color GetColor()
	{
		return color;
	}

	void OnTriggerStay(BoxCollider& other) override final
	{
		std::cout << "On Trigger Stay" << std::endl;
	}
private:
	std::vector<SerializableVariable> variables;
	Color color;
	std::string colorString;

	Sprite* sprite;
};