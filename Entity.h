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
	}

	inline void OnCollisionEnter(BoxCollider& other)
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

	inline void OnTriggerEnter(BoxCollider& other)
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
	inline void OnTriggerStay(BoxCollider& other)
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
	inline void OnTriggerExit(BoxCollider& other)
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

	inline void OnCollisionExit(BoxCollider& other)
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
			if (e->GetSerializedFields() == nullptr)
				continue;


			std::cout << e->GetSerializedFields()->size() << std::endl;
			ImGui::BeginChild(str.c_str());

			ImGui::Text(str.c_str());
			for (auto it = e->GetSerializedFields()->begin(); it != e->GetSerializedFields()->end(); ++it) {
				ImGui::Text(it->name);
				switch (it->type)
				{
				case Int_Type:
					break;
				case Float_Type:
				{
					float temp = it->read();
					ImGui::InputFloat(it->name, &temp);
					it->assign(temp);
					break;
				}
				case Char_Type:
				{
					auto temp1 = it->data;
					auto temp = *((char*)temp1);
					ImGui::InputText(it->name, &temp, 50);
					break;
				}
				case Bool_Type:
					break;
				default:
					assert(0);
				}
			}
			ImGui::EndChild();
		}
	}


private:
	bool isActive;

	ComponentList componentsList;
	ComponentBitset componentsBitset;

	std::string entityName;

	std::vector<std::unique_ptr<Component>> components;
};