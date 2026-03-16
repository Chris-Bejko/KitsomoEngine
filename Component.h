#pragma once

#include "Serialization.h"
#include <vector>

class Entity;
class Collider;

class Component
{
public:
	Component() = default;
	virtual ~Component() = default;
	Entity* entity;

	virtual bool Init() { return true; }

	virtual void Awake() { }
	virtual void draw() { }
	virtual void update(float dt) { }
	virtual void updateEngine(float dt) { }
	virtual std::vector<SerializableVariable>* GetSerializedFields() { return nullptr; }
	virtual void SetSerializedFields(std::vector<SerializableVariable> variables) { }
	virtual void InitSerializedFields(ReadableSerializableVariableMap map) { }


	virtual void OnCollisionEnter(Collider& other) { }
	virtual void OnCollisionExit(Collider& other) { }
	virtual void OnTriggerEnter(Collider& other) { }
	virtual void OnTriggerExit(Collider& other) { }
	virtual void OnTriggerStay(Collider& other) { }
	virtual void DrawEditorButton() {}
};