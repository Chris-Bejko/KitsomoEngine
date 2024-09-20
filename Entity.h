#pragma once

#include "ECS.h"
#include <vector>
#include <memory>
#include "Component.h"
#include "Transform.h"
#include "imgui.h"
#include "imgui-SFML.h"
#include <regex>

class Entity
{
public:
	bool displayComponents;
	Transform* transform;
	Entity(std::string name)
	{
		this->transform = &this->AddComponent<Transform>(0, 0);
		isActive = true;
		this->entityName = name;
	}
	virtual ~Entity() {}

	template <typename T, typename... TArgs>
	inline T& AddComponent(TArgs &&...args)
	{
		T* comp(new T(std::forward<TArgs>(args)...));
		std::unique_ptr<Component> uptr{ comp };
		components.emplace_back(std::move(uptr));

		comp->entity = this;

		if (comp->Init())
		{
			componentsList[getComponentTypeID<T>()] = comp;
			componentsBitset[getComponentTypeID<T>()] = true;
			return *comp;
		}

		return *static_cast<T*>(nullptr);
	}

	template <typename T>
	inline T& GetComponent() const
	{
		auto ptr(componentsList[getComponentTypeID<T>()]);
		return *static_cast<T*>(ptr);
	}

	template <typename T>
	inline bool HasComponent() const
	{
		return componentsBitset[getComponentTypeID<T>()];
	}

	inline bool IsActive() const
	{
		return isActive;
	}

	inline void Destroy()
	{
		isActive = false;
	}

	inline void Draw()
	{
		for (auto& comp : components)
		{
			comp->draw();
		}
	}

	inline void Update(float dt)
	{
		for (auto& comp : components)
		{
			comp->update(dt);
		}

		//if (!displayComponents)
			//return;

		//DisplayComponents();
	}

	inline void OnCollisionEnter(BoxCollider2D& other)
	{
		if (&other == nullptr)
			return;
		for (auto& comp : components)
		{
			if (!comp || !&other)
				continue;
			comp->OnCollisionEnter(other);
		}

	}

	inline void OnTriggerEnter(BoxCollider2D& other)
	{
		if (&other == nullptr)
			return;

		for (auto& comp : components)
		{
			if (!comp || !&other)
				continue;
			comp->OnTriggerEnter(other);
		}
	}
	inline void OnTriggerStay(BoxCollider2D& other)
	{
		if (&other == nullptr)
			return;

		for (auto& comp : components)
		{
			if (!comp || !&other)
				continue;
			comp->OnTriggerStay(other);
		}
	}
	inline void OnTriggerExit(BoxCollider2D& other)
	{
		if (&other == nullptr)
			return;

		for (auto& comp : components)
		{
			if (!comp || !&other)
				continue;
			comp->OnTriggerExit(other);
		}
	}

	inline void OnCollisionExit(BoxCollider2D& other)
	{
		if (&other == nullptr)
			return;

		for (auto& comp : components)
		{
			if (!comp || !&other)
				continue;
			comp->OnCollisionExit(other);
		}
	}

	inline std::string GetName()
	{
		return entityName;
	}

	inline void DisplayComponents()
	{
		if (!displayComponents)
			return;

		ImGui::Text(entityName.c_str());
		for (auto& e : components)
		{
			std::string str(typeid(*e).name());
			str = std::regex_replace(str, std::regex("class "), "");
			ImGui::Text(str.c_str());
		}
	}


private:
	bool isActive;

	ComponentList componentsList;
	ComponentBitset componentsBitset;

	std::string entityName;

	std::vector<std::unique_ptr<Component>> components;
};