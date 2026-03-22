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
#include "Components/PolygonCollider.h"
#include "Components/AudioSource.h"
#include "UI/UIRect.h"
#include "Components/UIButton.h"
#include "Components/UIImage.h"
#include "Components/UIText.h"
#include "Components/Canvas.h"
#include "Components/GameManager.h"
#include "Logger.h"
#include "EditorSprite.h"
#include "GUIDGenerator.h"
// #include "Components/BoxCollider.h"

Entity::Entity(std::string name, std::string guid)
{
	this->transform = &this->AddComponent<Transform>(0, 0);
	this->AddComponent<EditorSprite>();
	if (guid.empty())
	{
		this->m_guid = EngineGUID::Generate();
	}
	else
	{
		m_guid = guid;
	}
	isActive = true;
	displayComponents = false;
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

	if (parent)
	{
		parent->RemoveChild(this);
		parent = nullptr;
	}

	for (auto *child : children)
	{
		child->parent = nullptr;
		child->transform->SetParent(nullptr);
	}
	children.clear();
}

void Entity::Awake()
{
	ValidateAddedComponents();
	for (auto &comp : components)
	{	
		comp->ResolvePointers();
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
	for (auto &comp : components)
	{
		comp->draw();
	}
}

void Entity::Update(float dt)
{
	ValidateAddedComponents();
	for (auto &comp : components)
	{
		comp->update(dt);
	}
}

void Entity::UpdateEngine(float dt)
{
	ValidateAddedComponents();
	for (auto &comp : components)
	{
		comp->updateEngine(dt);
	}
}

void Entity::OnCollisionEnter(Collider &other)
{
	ValidateAddedComponents();
	if (&other == nullptr)
		return;
	for (auto &comp : components)
	{
		if (!comp || !&other)
			continue;
		comp->OnCollisionEnter(other);
	}
}

void Entity::OnTriggerEnter(Collider &other)
{
	ValidateAddedComponents();
	if (&other == nullptr)
		return;

	for (auto &comp : components)
	{
		if (!comp || !&other)
			continue;
		comp->OnTriggerEnter(other);
	}
}

void Entity::OnTriggerStay(Collider &other)
{
	ValidateAddedComponents();
	if (&other == nullptr)
		return;

	for (auto &comp : components)
	{
		if (!comp || !&other)
			continue;
		comp->OnTriggerStay(other);
	}
}

void Entity::OnTriggerExit(Collider &other)
{
	ValidateAddedComponents();
	if (&other == nullptr)
		return;

	for (auto &comp : components)
	{
		if (!comp || !&other)
			continue;
		comp->OnTriggerExit(other);
	}
}

void Entity::OnCollisionExit(Collider &other)
{
	ValidateAddedComponents();
	if (&other == nullptr)
		return;

	for (auto &comp : components)
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
	return entityName;
	return std::string();
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
	for (const auto &entry : std::filesystem::directory_iterator(path))
	{
		std::string path_string{entry.path().u8string()};
		availableComponents.push_back(path_string);
	}
}

void Entity::DisplayComponents()
{
	if (!displayComponents)
		return;

	// Delete entity button
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
	if (ImGui::Button("Delete Entity", ImVec2(-1, 0)))
		deletePressed = true;
	ImGui::PopStyleColor();

	if (deletePressed == 1)
	{
		std::string warning = "Delete Entity: " + this->GetName() + " ?";
		char *warningChar = &warning[0];
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
	Component *toRemove = nullptr;

	// Track component type counts for duplicate handling
	std::map<std::string, int> componentTypeCount;

	for (auto &e : components)
	{
		std::string str(typeid(*e).name());
		str = std::regex_replace(str, std::regex("class "), "");
		if (e->GetSerializedFields() == nullptr)
			continue;

		if (str == "Transform")
		{
			auto *t = dynamic_cast<Transform *>(e.get());
			if (t && t->isUITransform)
				continue; // skip rendering
		}
		// Increment count for this component type
		int componentIndex = componentTypeCount[str]++;

		// Create unique ID for ImGui (append index if multiple of same type exist)
		std::string uniqueId = str;
		if (componentIndex > 0)
			uniqueId += " (" + std::to_string(componentIndex + 1) + ")";

		// Component header
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
		ImGui::BeginChild((uniqueId + "##child").c_str(), ImVec2(0, 0), true);

		// Component name centered
		auto windowWidth = ImGui::GetWindowSize().x;
		auto textWidth = ImGui::CalcTextSize(uniqueId.c_str()).x;
		ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
		ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), uniqueId.c_str());

		// Remove button (skip Transform, it's required)
		if (str != "Transform" && str != "UIRect") // UIRect is also required for UI elements
		{
			ImGui::SameLine();
			ImGui::SetCursorPosX(windowWidth - 60);
			std::string removeLabel = "X##remove_" + uniqueId;
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
			std::string fieldId = std::string(it->name) + "##" + uniqueId;
			switch (it->type)
			{
			case int_Type:
			{
				auto p = reinterpret_cast<int *>(it->data);
				int temp(*p);
				ImGui::InputInt(fieldId.c_str(), &temp);
				*p = temp;
				if (auto *script = dynamic_cast<SerializableScript *>(e.get()))
					script->NotifyFieldChanged(std::string(it->name));
				break;
			}
			case float_Type:
			{
				auto p = reinterpret_cast<float *>(it->data);
				float temp(*p);
				ImGui::InputFloat(fieldId.c_str(), &temp);
				*p = temp;
				if (auto *script = dynamic_cast<SerializableScript *>(e.get()))
					script->NotifyFieldChanged(std::string(it->name));
				break;
			}
			case char_Type:
			{
				auto p = reinterpret_cast<std::string *>(it->data);
				std::string str(*p);
				str.resize(200, '\0');
				ImGui::InputText(fieldId.c_str(), &str[0], 200);
				*p = std::string(str.c_str());
				if (auto *script = dynamic_cast<SerializableScript *>(e.get()))
					script->NotifyFieldChanged(std::string(it->name));
				break;
			}
			case bool_Type:
			{
				auto p = reinterpret_cast<bool *>(it->data);
				bool b(*p);
				ImGui::Checkbox(fieldId.c_str(), &b);
				*p = b;
				if (auto *script = dynamic_cast<SerializableScript *>(e.get()))
					script->NotifyFieldChanged(std::string(it->name));
				break;
			}
			case entityRef_Type:
			{
				auto p = reinterpret_cast<std::string *>(it->data);

				// Find resolved entity by GUID
				Entity *resolved = nullptr;
				for (auto &ent : Engine::get().GetManager()->GetEntities())
				{
					if (ent->GetGUID() == *p)
					{
						resolved = ent.get();
						break;
					}
				}

				std::string displayText = resolved ? "-> " + resolved->GetName() : !p->empty() ? "NOT FOUND"
																							   : "Drop entity here...";

				ImVec4 boxColor = resolved ? ImVec4(0.1f, 0.25f, 0.1f, 1.0f) : !p->empty() ? ImVec4(0.35f, 0.1f, 0.1f, 1.0f)
																						   : ImVec4(0.15f, 0.18f, 0.25f, 1.0f);

				ImGui::PushStyleColor(ImGuiCol_Button, boxColor);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(boxColor.x + 0.05f,
																	 boxColor.y + 0.05f,
																	 boxColor.z + 0.1f, 1.0f));
				ImGui::Button((displayText + "##" + fieldId).c_str(), ImVec2(-1, 0));
				ImGui::PopStyleColor(2);

				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("ENTITY"))
					{
						Entity *dropped = *(Entity **)payload->Data;
						if (dropped)
							*p = dropped->GetGUID(); // just store GUID directly
					}
					ImGui::EndDragDropTarget();
				}
				break;
			}
			case compRef_Type:
			{
				auto p = reinterpret_cast<std::string *>(it->data);
				std::string typeHint = it->componentTypeHint;

				// Parse packed string "entityGUID|compGUID"
				std::string entityGUID, compGUID;
				auto pipePos = p->find('|');
				if (pipePos != std::string::npos)
				{
					entityGUID = p->substr(0, pipePos);
					compGUID = p->substr(pipePos + 1);
				}

				// Find resolved entity by GUID
				Entity *resolved = nullptr;
				for (auto &ent : Engine::get().GetManager()->GetEntities())
				{
					if (ent->GetGUID() == entityGUID)
					{
						resolved = ent.get();
						break;
					}
				}

				// Build display text: "UIText (EntityName)"
				std::string displayText = "Drop component here...";
				if (resolved)
					displayText = typeHint + " (" + resolved->GetName() + ")";
				else if (!entityGUID.empty())
					displayText = "NOT FOUND";

				ImVec4 boxColor = resolved ? ImVec4(0.1f, 0.25f, 0.1f, 1.0f) : !entityGUID.empty() ? ImVec4(0.35f, 0.1f, 0.1f, 1.0f)
																								   : ImVec4(0.15f, 0.18f, 0.25f, 1.0f);

				ImGui::PushStyleColor(ImGuiCol_Button, boxColor);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(boxColor.x + 0.05f,
																	 boxColor.y + 0.05f,
																	 boxColor.z + 0.1f, 1.0f));
				ImGui::Button((displayText + "##" + fieldId).c_str(), ImVec2(-1, 0));
				ImGui::PopStyleColor(2);

				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("ENTITY"))
					{
						Entity *dropped = *(Entity **)payload->Data;
						if (dropped)
						{
							// Find component matching type hint
							Component *matchedComp = nullptr;
							for (auto &comp : dropped->GetComponents())
							{
								std::string compTypeName(typeid(*comp).name());
								compTypeName = std::regex_replace(compTypeName,
																  std::regex("class "), "");
								if (compTypeName == typeHint)
								{
									matchedComp = comp.get();
									break;
								}
							}

							if (matchedComp)
								*p = dropped->GetGUID() + "|" + matchedComp->GetGUID();
							// if no match, don't accept
						}
					}
					ImGui::EndDragDropTarget();
				}
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
void Entity::RemoveComponent(Component *comp)
{
	for (size_t i = 0; i < componentsList.size(); i++)
	{
		if (componentsList[i] == comp)
		{
			componentsList[i] = nullptr;
			componentsBitset[i] = false;
			break;
		}
	}

	components.erase(
		std::remove_if(components.begin(), components.end(),
					   [comp](const std::unique_ptr<Component> &c)
					   {
						   return c.get() == comp;
					   }),
		components.end());
}
void Entity::DisplayAvailableComponents()
{
	if (addingNewComp)
	{
		ImGui::BeginChild("Add Component");
		for (auto &comp : availableComponents)
		{
			std::string str(comp);
			str = std::regex_replace(str, std::regex("Components/"), "");
			str = std::regex_replace(str, std::regex(".h"), "");
			if (str.find(".cpp") != std::string::npos)
			{
				continue;
			}
			auto temp = str.c_str();
			if (ImGui::Button(temp))
			{
				AddComponentByName(str);
				addingNewComp = false;
			}
		}
		ImGui::EndChild();
	}
}

// Component factory - create and add component by name
void Entity::AddComponentByName(const std::string &componentName)
{
	// For single-instance components, check if already exists
	bool allowsMultiple = ComponentAllowsMultiple(componentName);
	if (!allowsMultiple)
	{
		for (auto &comp : components)
		{
			std::string str(typeid(*comp).name());
			str = std::regex_replace(str, std::regex("class "), "");
			if (str == componentName)
				return; // Already exists, don't add again
		}
	}

	if (componentName == "Transform")
		this->AddComponent<Transform>();
	else if (componentName == "BoxCollider")
		this->AddComponent<BoxCollider>();
	else if (componentName == "Sprite")
		this->AddComponent<Sprite>();
	else if (componentName == "Player")
		this->AddComponent<Player>();
	else if (componentName == "FloorSquare")
		this->AddComponent<FloorSquare>();
	else if (componentName == "Bullet")
		this->AddComponent<Bullet>();
	else if (componentName == "Rigidbody")
		this->AddComponent<Rigidbody>();
	else if (componentName == "CircleCollider")
		this->AddComponent<CircleCollider>();
	else if (componentName == "PolygonCollider")
		this->AddComponent<PolygonCollider>();
	else if (componentName == "AudioSource")
		this->AddComponent<AudioSource>();
	else if (componentName == "UIButton")
		this->AddComponent<UIButton>();
	else if (componentName == "UIImage")
		this->AddComponent<UIImage>();
	else if (componentName == "UIText")
		this->AddComponent<UIText>();
	else if (componentName == "Canvas")
		this->AddComponent<Canvas>();
	else if (componentName == "GameManager")
		this->AddComponent<GameManager>();
	else
		LOG_ERROR("Unknown component: ", componentName.c_str());
}

// Static metadata map defining which components allow multiple instances
bool Entity::ComponentAllowsMultiple(const std::string &componentName)
{
	// Colliders allow multiple instances
	if (componentName == "BoxCollider" ||
		componentName == "CircleCollider" ||
		componentName == "PolygonCollider")
		return true;

	// All others allow only one instance
	return false;
}

std::vector<SerializableComponent> Entity::GetAllComponentVariables()
{
	std::vector<SerializableComponent> variables;
	for (auto &c : components)
	{
		if (c->GetSerializedFields() != nullptr)
		{
			SerializableComponent ser;
			std::string str(typeid(*c).name());
			str = std::regex_replace(str, std::regex("class "), "");
			ser.componentName = str;
			ser.variables = *c->GetSerializedFields();

			// Also build the fields map from variables
			for (const auto &var : ser.variables)
			{
				if (var.name && var.data)
				{
					switch (var.type)
					{
					case int_Type:
						ser.fields.intFields[var.name] = *(int *)var.data;
						break;
					case float_Type:
						ser.fields.floatFields[var.name] = *(float *)var.data;
						break;
					case char_Type:
						ser.fields.stringFields[var.name] = *reinterpret_cast<std::string *>(var.data);
						break;
					case bool_Type:
						ser.fields.boolFields[var.name] = *(bool *)var.data;
						break;
					}
				}
			}

			variables.push_back(ser);
		}
	}
	return variables;
}

void Entity::InitializeComponentFields(const std::vector<SerializableComponent> &serializedComps)
{
	ValidateAddedComponents();

	// Match components by type name to handle any order differences
	for (const auto &serialComp : serializedComps)
	{
		LOG_DEBUG("Initializing component: ", serialComp.componentName.c_str());
		LOG_DEBUG("  float fields count: ", serialComp.fields.floatFields.size());
		for (auto const &[key, value] : serialComp.fields.floatFields)
			LOG_DEBUG("  float field: ", key.c_str(), " = ", value);
		// Find the component in this entity that matches the serialized component's type
		for (auto &comp : components)
		{
			if (comp)
			{
				// Get the type name of the component
				std::string compTypeName(typeid(*comp).name());
				compTypeName = std::regex_replace(compTypeName, std::regex("class "), "");

				// If names match, initialize this component with the serialized fields
				if (compTypeName == serialComp.componentName)
				{
					comp->InitSerializedFields(serialComp.fields);
					break; // Found and initialized, move to next serialized component
				}
			}
		}
	}
	ValidateAddedComponents();
}

bool Entity::DeletePressed()
{
	return deletePressed;
}

void Entity::SetParent(Entity *newParent)
{
	if (parent)
		parent->RemoveChild(this);

	parent = newParent;

	if (parent)
	{
		parent->AddChild(this);
		// Link transforms
		transform->SetParent(parent->transform);
	}
	else
	{
		transform->SetParent(nullptr);
	}
}

void Entity::AddChild(Entity *child)
{
	children.push_back(child);
}

void Entity::RemoveChild(Entity *child)
{
	children.erase(
		std::remove(children.begin(), children.end(), child),
		children.end());
}