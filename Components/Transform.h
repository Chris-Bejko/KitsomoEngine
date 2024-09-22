#pragma once
#include "../Component.h"
#include "../Vector2.h"

#define DECLARE(TYPE, NAME, VALUE) \
variables.push_back({NAME, #NAME, TYPE})
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
		variables.push_back({ "position.y", &position.y, float_Type });
		variables.push_back({ "rotation", &rotation, float_Type });
		variables.push_back({ "scale.x", &scale.x, float_Type });
		variables.push_back({ "scale.y", &scale.y, float_Type });
		variables.push_back({ "tag", &tag, char_Type });
		variables.push_back({ "testBool", &testBool, bool_Type });
	}

	void InitSerializedFields(ReadableSerializableVariableMap variables)
	{
		for (auto const& [key, value] : variables.floatFields)
		{
			if(key == "position.x")
			{
				position.x = value;
				//this->variables.push_back({ f.name , &position.x, float_Type });
			}
			if (key == "position.y")
			{
				position.y = value;
				//this->variables.push_back({ f.name , &position.y, float_Type });
			}
			if (key == "rotation")
			{
				rotation = value;
				//this->variables.push_back({ f.name , &rotation, float_Type });
			}
			if (key == "scale.x")
			{
				scale.x = value;
				//this->variables.push_back({ f.name , &scale.x, float_Type });
			}
			if (key == "scale.y")
			{
				scale.y = value;
				//this->variables.push_back({ f.name , &scale.y, float_Type });
			}
		}

		for(auto const& [key, value] : variables.stringFields)
		{
			if(key == "tag")
			{
				tag = value;
			}
		}

		for(auto const& [key, value] : variables.boolFields)
		{
			if(key == "testBool")
			{
				testBool = value;
			}
		}
	}
	std::vector<SerializableVariable>* GetSerializedFields() override final
	{
		return &variables;
	}

	bool Init() override final
	{
		//tag = "Test Tag";
		SerializeVariables();
		return true;
	}
	Vector2F position = Vector2F();
	Vector2F scale = Vector2F(1.0f, 1.0f);
	float rotation = 0.0f;
	std::string tag;
	bool testBool;

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