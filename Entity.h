#pragma once

#include "ECS.h"
#include <vector>
#include <memory>
#include "Component.h"
#include "Components/Transform.h"

#include "imgui.h"
#include "imgui-SFML.h"
#include <regex>
#include <filesystem>
#include <iostream>

class Entity
{
public:
	bool displayComponents;
	Transform* transform;
	Entity(std::string name)
	{
		this->transform = &this->AddComponent<Transform>(0, 0);
		isActive = true;
		SaveAvailableComponents();
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
		if (entityName.empty())
			return "##";
		return entityName;
	}

	inline void SetName(std::string name)
	{
		if (name.empty())
			entityName = "##";
		else
			entityName = name;
	}

	inline void SaveAvailableComponents()
	{
		std::string path = "Components/";
		for (const auto& entry : std::filesystem::directory_iterator(path))
		{
			std::string path_string{ entry.path().u8string() };
			availableComponents.push_back(path_string);
		}
	}

	inline void DisplayComponents()
	{
		if (!displayComponents)
			return;

		ImGui::Checkbox("+", &addingNewComp);

		auto windowWidth = ImGui::GetWindowSize().x;
		auto textWidth = ImGui::CalcTextSize(GetName().c_str()).x;

		ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
		auto temp = GetName();
		auto temp1 = &temp[0];
		ImGui::InputText("Name", temp1, 55);
		SetName(temp);
		for (auto& e : components)
		{
			std::string str(typeid(*e).name());
			str = std::regex_replace(str, std::regex("class "), "");
			if (e->GetSerializedFields() == nullptr)
				continue;

			ImGui::BeginChild(str.c_str());
			auto windowWidth = ImGui::GetWindowSize().x;
			auto textWidth = ImGui::CalcTextSize(str.c_str()).x;

			ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
			ImGui::Text(str.c_str());
			for (auto it = e->GetSerializedFields()->begin(); it != e->GetSerializedFields()->end(); ++it) {
				ImGui::Text(it->name);
				switch (it->type)
				{
				case int_Type:
				{
					int temp = it->read();
					ImGui::InputInt(it->name, &temp);
					it->assign(temp);
					break;
				}
				case float_Type:
				{
					float temp = it->read();
					ImGui::InputFloat(it->name, &temp);
					it->assign(temp);
					break;
				}
				case char_Type:
				{
					auto temp1 = it->data;
					auto temp = *((char*)temp1);
					ImGui::InputText(it->name, &temp, 50);
					break;
				}
				case bool_Type:
				{
					auto temp1 = it->data;
					auto temp = *((bool*)temp1);
					//ImGui::Checkbox(it->name, &temp);
					break;
				}
				default:
					assert(0);
				}
			}
			ImGui::EndChild();
		}
		DisplayAvailableComponents();


	}

	void DisplayAvailableComponents()
	{
		if (addingNewComp)
		{
			ImGui::BeginChild("Add Component");
			for (auto& comp : availableComponents)
			{
				std::string str(comp);
				str = std::regex_replace(str, std::regex("Components/"), "");
				str = std::regex_replace(str, std::regex(".h"), "");
				auto temp = str.c_str();
				if (ImGui::Button(temp))
				{
					if(str == "Transform")
					{
						std::cout << "Adding player" << std::endl;
						this->AddComponent<Transform>();
					}
				}
			}
			ImGui::EndChild();
		}
	}

	std::vector<SerializableComponent> GetAllComponentVariables()
	{
		std::vector<SerializableComponent> variables;
		for(auto& c : components)
		{
			if(c->GetSerializedFields() != nullptr)
			{
				SerializableComponent ser;
				std::string str(typeid(*c).name());
				str = std::regex_replace(str, std::regex("class "), "");
				ser.componentName = str;
				ser.fields = *c->GetSerializedFields();
				variables.push_back(ser);
			}
		}
		return variables;
	}


private:
	bool isActive;

	std::vector<std::string> availableComponents;
	bool addingNewComp = false;
	ComponentList componentsList;
	ComponentBitset componentsBitset;

	std::string entityName;

	std::vector<std::unique_ptr<Component>> components;
};