#pragma once
#include "../Component.h"
#include "../Vector2.h"

#define DECLARE(TYPE, NAME, VALUE) \
variables.push_back({NAME, #NAME, TYPE})
class Transform : public Component
{
public:
	Transform() = default;
	Transform(float x, float y);
	Transform(float x, float y, float scX, float scY);

	Transform(float x, float y, float scX, float scY, float roation);
	virtual ~Transform() = default;

	void SerializeVariables();

	void InitSerializedFields(ReadableSerializableVariableMap variables);

	std::vector<SerializableVariable>* GetSerializedFields() override final;

	bool Init() override final;

	Vector2F position = Vector2F();
	Vector2F scale = Vector2F(1.0f, 1.0f);
	float rotation = 0.0f;

	void Translate(const Vector2F velocity);

	void LookAt(const float rotation);

	void SetPosition(float x, float y);

private:
	std::vector<SerializableVariable> variables;
};