#include "Entity.h"
#include <vector>
#include "Components/Transform.h"
#include "Components/Sprite.h"
#include "Components/Player.h"
//#include "Components/BoxCollider.h"

Entity::Entity(std::string name)
{
	this->transform = &this->AddComponent<Transform>(0, 0);
	isActive = true;
	SaveAvailableComponents();
	this->entityName = name;
}

bool Entity::IsActive() const
{
	return isActive;
}

void Entity::Destroy()
{
	isActive = false;
}

void Entity::Awake()
{
	for (auto& comp : components)
	{
		comp->Awake();
	}
}

void Entity::Draw()
{
	for (auto& comp : components)
	{
		comp->draw();
	}
}

void Entity::Update(float dt)
{
	for (auto& comp : components)
	{
		comp->update(dt);
	}
}

void Entity::UpdateEngine(float dt)
{
	for(auto& comp : components)
	{
		comp->updateEngine(dt);
	}
}

void Entity::OnCollisionEnter(BoxCollider& other)
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

void Entity::OnTriggerEnter(BoxCollider& other)
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

void Entity::OnTriggerStay(BoxCollider& other)
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

void Entity::OnTriggerExit(BoxCollider& other)
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

void Entity::OnCollisionExit(BoxCollider& other)
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

std::string Entity::GetName()
{
	if (entityName.empty())
		return "##";
	return entityName;	return std::string();
}

void Entity::SetName(std::string name)
{
	if (name.empty())
		entityName = "##";
	else
		entityName = name;
}

void Entity::SaveAvailableComponents()
{
	std::string path = "Components/";
	for (const auto& entry : std::filesystem::directory_iterator(path))
	{
		std::string path_string{ entry.path().u8string() };
		availableComponents.push_back(path_string);
	}
}

void Entity::DisplayComponents()
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
				auto p = reinterpret_cast<int*>(it->data);
				int temp(*p);
				ImGui::InputInt(it->name, &temp);
				*p = temp;
				break;
			}
			case float_Type:
			{
				auto p = reinterpret_cast<float*>(it->data);
				float temp(*p);
				ImGui::InputFloat(it->name, &temp);
				*p = temp;
				break;
			}
			case char_Type:
			{

				auto p = reinterpret_cast<std::string*>(it->data);
				std::string str(*p);
				ImGui::InputText(it->name, &str[0], 50);
				*p = &str[0];
				break;
			}
			case bool_Type:
			{
				auto p = reinterpret_cast<bool*>(it->data);
				bool b(*p);
				ImGui::Checkbox(it->name, &b);
				*p = b;
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

void Entity::DisplayAvailableComponents()
{
	if (addingNewComp)
	{
		ImGui::BeginChild("Add Component");
		for (auto& comp : availableComponents)
		{
			std::string str(comp);
			str = std::regex_replace(str, std::regex("Components/"), "");
			str = std::regex_replace(str, std::regex(".h"), "");
			if (str.find(".cpp") != std::string::npos) {
				continue;
			}
			auto temp = str.c_str();
			if (ImGui::Button(temp))
			{
				std::cout << temp << std::endl;
				if (str == "Transform")
				{
					if (!this->HasComponent<Transform>())
						this->AddComponent<Transform>();
				}
				else if (str == "BoxCollider")
				{
					std::cout << "should add box collider" << std::endl;
					if (!this->HasComponent<BoxCollider>())
					{
						this->AddComponent<BoxCollider>();
					}
				}
				else if (str == "Sprite")
				{
					std::cout << "Should add sprite" << std::endl;
					if (!this->HasComponent<Sprite>())
						this->AddComponent<Sprite>();
				}
				else if (str == "Player")
				{
					if (!this->HasComponent<Player>())
						this->AddComponent<Player>();
				}
			}
		}
		ImGui::EndChild();
	}
}

std::vector<SerializableComponent> Entity::GetAllComponentVariables()
{
	std::vector<SerializableComponent> variables;
	for (auto& c : components)
	{
		if (c->GetSerializedFields() != nullptr)
		{
			SerializableComponent ser;
			std::string str(typeid(*c).name());
			str = std::regex_replace(str, std::regex("class "), "");
			ser.componentName = str;
			ser.variables = *c->GetSerializedFields();
			variables.push_back(ser);
		}
	}
	return variables;
}


