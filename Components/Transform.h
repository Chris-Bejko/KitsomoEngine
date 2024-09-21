#pragma once
#include "../Component.h"
#include "../Vector2.h"

#define DECLARE(TYPE, NAME, VALUE) \
variables.push_back({#NAME, & NAME, TYPE})
class Transform : public Component
{
public:
	Transform() = default;
	Transform(float x, float y)
	{
		position.x = x;
		position.y = y;
	}

	Transform(float x, float y, float scX, float scY)
	{
		position.x = x;
		position.y = y;
		scale.x = scX;
		scale.y = scY;
		rotation = 0;
	}

	Transform(float x, float y, float scX, float scY, float roation)
	{
		position.x = x;
		position.y = y;
		scale.x = scX;
		scale.y = scY;
		this->rotation = rotation;
	}
	virtual ~Transform() = default;

	void SerializeVariables()
	{
		variables.push_back({ "position.x", &position.x, float_Type });
		variables.push_back({ "position.y", &position.x, float_Type });
		variables.push_back({ "rotation", &rotation, float_Type });
		variables.push_back({ "scale.x", &scale.x, float_Type });
		variables.push_back({ "scale.y", &scale.y, float_Type });
	}

	void SetSerializedFields(std::vector<SerializableVariable> variables) override final
	{
		for (auto& f : variables)
		{
			DECLARE(f.type, f.name, f.read());
		}
	}
	std::vector<SerializableVariable>* GetSerializedFields() override final
	{
		return &variables;
	}

	bool Init() override final
	{
		SerializeVariables();
		return true;
	}
	Vector2F position = Vector2F();
	Vector2F scale = Vector2F(1.0f, 1.0f);
	float rotation = 0.0f;

	void Translate(const Vector2F velocity)
	{
		position += velocity;
	}

	void LookAt(const float rotation)
	{
		this->rotation = rotation;
	}
private:
	std::vector<SerializableVariable> variables;
};