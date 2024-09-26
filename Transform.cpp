#include "Components/Transform.h"

Transform::Transform(float x, float y)
{
	position.x = x;
	position.y = y;
}

Transform::Transform(float x, float y, float scX, float scY)
{
	position.x = x;
	position.y = y;
	scale.x = scX;
	scale.y = scY;
	rotation = 0;
}

Transform::Transform(float x, float y, float scX, float scY, float roation)
{
	position.x = x;
	position.y = y;
	scale.x = scX;
	scale.y = scY;
	this->rotation = rotation;
}

void Transform::SerializeVariables()
{
	variables.push_back({ "position.x", &position.x, float_Type });
	variables.push_back({ "position.y", &position.y, float_Type });
	variables.push_back({ "rotation", &rotation, float_Type });
	variables.push_back({ "scale.x", &scale.x, float_Type });
	variables.push_back({ "scale.y", &scale.y, float_Type });
}

void Transform::InitSerializedFields(ReadableSerializableVariableMap variables)
{
	for (auto const& [key, value] : variables.floatFields)
	{
		if (key == "position.x")
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
}

std::vector<SerializableVariable>* Transform::GetSerializedFields()
{
	return &variables;
}

bool Transform::Init()
{
	//tag = "Test Tag";
	SerializeVariables();
	return true;
}

void Transform::Translate(const Vector2F velocity)
{
	position += velocity;
}

void Transform::LookAt(const float rotation)
{
	this->rotation = rotation;
}

void Transform::SetPosition(float x, float y)
{
	position.x = x;
	position.y = y;
}