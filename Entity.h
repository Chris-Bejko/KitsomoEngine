#pragma once

#include "ECS.h"
#include <vector>
#include <memory>
#include "imgui.h"
#include "imgui-SFML.h"
#include <regex>
#include "Component.h"
#include <filesystem>
#include <iostream>
#include "Collision/Collider.h"

class Transform;
class Collider;
class SerializableComponent;

class Entity
{
public:
	bool displayComponents;
	Transform *transform;
	Entity(std::string name);

	virtual ~Entity() {}

	template <typename T, typename... TArgs>
	inline T &AddComponent(TArgs &&...args)
	{
		T *comp(new T(std::forward<TArgs>(args)...));
		std::unique_ptr<Component> uptr{comp};
		to_Add.emplace_back(std::move(uptr));

		comp->entity = this;
		if (comp->Init())
		{
			componentsList[getComponentTypeID<T>()] = comp;
			componentsBitset[getComponentTypeID<T>()] = true;
			return *comp;
		}

		return *static_cast<T *>(nullptr);
	}

	template <typename T>
	inline T &GetComponent() const
	{
		auto ptr(componentsList[getComponentTypeID<T>()]);
		return *static_cast<T *>(ptr);
	}

	template <typename T>
	inline bool HasComponent() const
	{
		return componentsBitset[getComponentTypeID<T>()];
	}

	template <typename T>
	inline bool HasComponentOfType() const
	{
		for (auto &comp : components)
		{
			if (dynamic_cast<T *>(comp.get()) != nullptr)
				return true;
		}
		return false;
	}

	template <typename T>
	inline T *GetComponentOfType() const
	{
		for (auto &comp : components)
		{
			T *casted = dynamic_cast<T *>(comp.get());
			if (casted != nullptr)
				return casted;
		}
		return nullptr;
	}
	void RemoveComponent(Component *comp);

	void Awake();

	bool IsActive() const;

	void Destroy();

	void ValidateAddedComponents();

	void Draw();

	void Update(float dt);

	void UpdateEngine(float dt);

	void OnCollisionEnter(Collider &other);

	void OnTriggerEnter(Collider &other);

	void OnTriggerStay(Collider &other);

	void OnTriggerExit(Collider &other);

	void OnCollisionExit(Collider &other);

	std::string GetName();

	void SetName(std::string name);

	void SaveAvailableComponents();

	void DisplayComponents();

	void DisplayAvailableComponents();

	std::vector<SerializableComponent> GetAllComponentVariables();

	bool DeletePressed();

	// Component factory - add component by name string
	void AddComponentByName(const std::string& componentName);

	// Check if a component type allows multiple instances
	static bool ComponentAllowsMultiple(const std::string& componentName);

	// Initialize all components with their serialized field data
	void InitializeComponentFields(const std::vector<SerializableComponent>& serializedComps);

	const std::vector<std::unique_ptr<Component>> &GetComponents() { return components; }

private:
	bool isActive;

	bool deletePressed = false;

	std::vector<std::string> availableComponents;
	bool addingNewComp = false;
	ComponentList componentsList;
	ComponentBitset componentsBitset;

	std::string entityName;

	std::vector<std::unique_ptr<Component>> components;
	std::vector<std::unique_ptr<Component>> to_Add;
};