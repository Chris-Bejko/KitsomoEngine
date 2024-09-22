#pragma once

#include "Serialization.h"
#include <vector>

class Entity;
class BoxCollider;

class Component
{
public:
	Component() = default;
	virtual ~Component() = default;
	Entity* entity;

	virtual bool Init() { return true; }
	virtual void draw() { }
	virtual void update(float dt) { }
	virtual std::vector<SerializableVariable>* GetSerializedFields() { return nullptr; }
	virtual void SetSerializedFields(std::vector<SerializableVariable> variables) { }


	virtual void OnCollisionEnter(BoxCollider& other) { }
	virtual void OnCollisionExit(BoxCollider& other) { }
	virtual void OnTriggerEnter(BoxCollider& other) { }
	virtual void OnTriggerExit(BoxCollider& other) { }
	virtual void OnTriggerStay(BoxCollider& other) { }
};