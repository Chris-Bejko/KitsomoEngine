#pragma once

#include "../Component.h"
#include "../Color.h"
#include "Sprite.h"

class FloorSquare : public Component
{
public:
	FloorSquare() = default;

	virtual ~FloorSquare() = default;
	bool Init() override final;

	void Config(Vector2F position, Color color);

	void Serialize();

	void InitSerializedFields(ReadableSerializableVariableMap map);

	void update(float dt) override final;

	void updateEngine(float dt) override final;

	std::vector<SerializableVariable>* GetSerializedFields() override final;

	Color GetColorEnum();

	void OnTriggerStay(BoxCollider& other) override final;

private:
	std::vector<SerializableVariable> variables;
	Color color;
	std::string colorString;

	Sprite* sprite;
};