#pragma once

#include "Serialization.h"
#include <vector>
#include "SerializedField.h"
#include <memory>
class Entity;
class Collider;

class Component
{
public:
	Component() = default;
	virtual ~Component() = default;
	Entity *entity;

	virtual bool Init() { return true; }

	virtual void Awake() {}
	virtual void draw() {}
	virtual void update(float dt) {}
	virtual void updateEngine(float dt) {}
	virtual const std::vector<std::unique_ptr<SerializedField>> &
	GetSerializedFields() const
	{
		static const std::vector<std::unique_ptr<SerializedField>> empty;
		return empty;
	}

	virtual void OnCollisionEnter(Collider &other) {}
	virtual void OnCollisionExit(Collider &other) {}
	virtual void OnTriggerEnter(Collider &other) {}
	virtual void OnTriggerExit(Collider &other) {}
	virtual void OnTriggerStay(Collider &other) {}
	virtual void DrawEditorButton() {}
	virtual void ResolvePointers() {}
	void SetGUID(std::string guid) { m_GUID = guid; }
	std::string GetGUID() { return m_GUID; }

private:
	std::string m_GUID;
};