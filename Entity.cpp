#include "Entity.h"
#include <vector>
#include "Components/Transform.h"
#include "Components/Sprite.h"
#include "Components/Player.h"
#include "Engine.h"
#include "Components/FloorSquare.h"
#include "Components/Bullet.h"
#include "Components/Rigidbody.h"
#include "Components/CircleCollider.h"
#include "Components/BoxCollider.h"

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
	ValidateAddedComponents();
	for (auto& comp : components)
	{
		comp->Awake();
	}
}

void Entity::ValidateAddedComponents()
{
	while (!to_Add.empty())
	{
		components.push_back(std::move(to_Add.back()));
		to_Add.pop_back();
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
	ValidateAddedComponents();
	for (auto& comp : components)
	{
		comp->update(dt);
	}
}

void Entity::UpdateEngine(float dt)
{
	ValidateAddedComponents();
	for (auto& comp : components)
	{
		comp->updateEngine(dt);
	}
}

void Entity::OnCollisionEnter(Collider& other)
{
	ValidateAddedComponents();
	if (&other == nullptr)
		return;
	for (auto& comp : components)
	{
		if (!comp || !&other)
			continue;
		comp->OnCollisionEnter(other);
	}

}

void Entity::OnTriggerEnter(Collider& other)
{
	ValidateAddedComponents();
	if (&other == nullptr)
		return;

	for (auto& comp : components)
	{
		if (!comp || !&other)
			continue;
		comp->OnTriggerEnter(other);
	}
}

void Entity::OnTriggerStay(Collider& other)
{
	ValidateAddedComponents();
	if (&other == nullptr)
		return;

	for (auto& comp : components)
	{
		if (!comp || !&other)
			continue;
		comp->OnTriggerStay(other);
	}
}

void Entity::OnTriggerExit(Collider& other)
{
	ValidateAddedComponents();
	if (&other == nullptr)
		return;

	for (auto& comp : components)
	{
		if (!comp || !&other)
			continue;
		comp->OnTriggerExit(other);
	}
}

void Entity::OnCollisionExit(Collider& other)
{
	ValidateAddedComponents();
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

    // Delete entity button
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
    if (ImGui::Button("-Delete Entity", ImVec2(-1, 0)))
        deletePressed = true;
    ImGui::PopStyleColor();

    if (deletePressed)
    {
        std::string warning = "Delete Entity: " + this->GetName() + " ?";
        char* warningChar = &warning[0];
        ImGui::OpenPopup(warningChar);
        if (ImGui::BeginPopupModal(warningChar))
        {
            ImGui::Text("This action cannot be undone.");
            ImGui::Separator();
            if (ImGui::Button("Cancel", ImVec2(120, 0)))
                deletePressed = false;
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
            if (ImGui::Button("Confirm", ImVec2(120, 0)))
            {
                Engine::get().RemoveEntity(this);
                deletePressed = false;
            }
            ImGui::PopStyleColor();
            ImGui::EndPopup();
        }
    }

    ImGui::Separator();

    // Entity name
    auto windowWidth = ImGui::GetWindowSize().x;
    auto textWidth = ImGui::CalcTextSize("Name").x;
    ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
    auto temp = GetName();
    temp.resize(55, '\0');
    ImGui::InputText("##name", &temp[0], 55);
    SetName(std::string(temp.c_str()));

    ImGui::Separator();

    // Add component button
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.4f, 0.1f, 1.0f));
    if (ImGui::Button("+ Add Component", ImVec2(-1, 0)))
        addingNewComp = !addingNewComp;
    ImGui::PopStyleColor();

    if (addingNewComp)
        DisplayAvailableComponents();

    ImGui::Separator();
    ValidateAddedComponents();

    // Component to remove (deferred to avoid modifying list while iterating)
    Component* toRemove = nullptr;

    for (auto& e : components)
    {
        std::string str(typeid(*e).name());
        str = std::regex_replace(str, std::regex("class "), "");
        if (e->GetSerializedFields() == nullptr)
            continue;

        // Component header
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
        ImGui::BeginChild(str.c_str(), ImVec2(0, 0), true);

        // Component name centered
        auto windowWidth = ImGui::GetWindowSize().x;
        auto textWidth = ImGui::CalcTextSize(str.c_str()).x;
        ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
        ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), str.c_str());

        // Remove button (skip Transform, it's required)
        if (str != "Transform")
        {
            ImGui::SameLine();
            ImGui::SetCursorPosX(windowWidth - 60);
            std::string removeLabel = "X##" + str;
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
            if (ImGui::Button(removeLabel.c_str(), ImVec2(50, 0)))
                toRemove = e.get();
            ImGui::PopStyleColor();
        }

        ImGui::Separator();

        // Fields
        for (auto it = e->GetSerializedFields()->begin(); it != e->GetSerializedFields()->end(); ++it)
        {
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), it->name);
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
                str.resize(200, '\0');
                ImGui::InputText(it->name, &str[0], 200);
                *p = std::string(str.c_str());
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
		e->DrawEditorButton();
        ImGui::EndChild();
        ImGui::PopStyleColor();
        ImGui::Spacing();
    }

    // Remove component after iteration
    if (toRemove != nullptr)
        RemoveComponent(toRemove);
}
void Entity::RemoveComponent(Component* comp)
{
    components.erase(
        std::remove_if(components.begin(), components.end(),
            [comp](const std::unique_ptr<Component>& c) {
                return c.get() == comp;
            }),
        components.end()
    );
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
				if (str == "Transform")
				{
					if (!this->HasComponent<Transform>())
						this->AddComponent<Transform>();
				}
				else if (str == "BoxCollider")
				{
					if (!this->HasComponent<BoxCollider>())
					{
						this->AddComponent<BoxCollider>();
					}
				}
				else if (str == "Sprite")
				{
					if (!this->HasComponent<Sprite>())
						this->AddComponent<Sprite>();
				}
				else if (str == "Player")
				{
					if (!this->HasComponent<Player>())
						this->AddComponent<Player>();
				}
				else if (str == "FloorSquare")
				{
					if (!this->HasComponent<FloorSquare>())
						this->AddComponent<FloorSquare>();
				}else if(str == "Bullet"){
					if(!this->HasComponent<Bullet>())
						this->AddComponent<Bullet>();
				}else if(str == "Rigidbody"){
					if(!this->HasComponent<Rigidbody>())
						this->AddComponent<Rigidbody>();
				}else if(str == "CircleCollider"){
					if(!this->HasComponent<CircleCollider>())
						this->AddComponent<CircleCollider>();
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

bool Entity::DeletePressed()
{
	return deletePressed;
}


