#include "Entity.h"
#include <vector>
#include "Transform.h"
#include "Sprite.h"
#include "Engine.h"
#include "Rigidbody.h"
#include "CircleCollider.h"
#include "BoxCollider.h"
#include "PolygonCollider.h"
#include "AudioSource.h"
#include "UIRect.h"
#include "UIButton.h"
#include "UIImage.h"
#include "UIText.h"
#include "Canvas.h"
#include "Logger.h"
#include "EditorSprite.h"
#include "GUIDGenerator.h"
#include "ComponentRegistry.h"
// #include "Components/BoxCollider.h"
#include <typeinfo>

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

bool Entity::IsActiveInHierarchy() const
{
	const Entity *current = this;
	int depth = 0;
	while (current != nullptr)
	{
		if (current->isPendingDestroy)
			return false;
		if (!current->isActive)
			return false;
		current = current->GetParent();
	}
	return true;
}

void Entity::Destroy()
{
	isActive = false;
	isPendingDestroy = true;
	Engine::get().QueueDestroy(GetGUID());
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
	availableComponents = ComponentRegistry::get().GetNames();
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
				Engine::get().QueueDestroy(GetGUID());
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

				// Check if this is a vector field
				SerializableScript *script = dynamic_cast<SerializableScript *>(e.get());
				if (script && (script->IsVectorField(std::string(it->name))))
				{
					DrawVectorField(script, it->name, fieldId);
					break;
				}

				// Normal string field
				auto str = *p;
				str.resize(200, '\0');
				ImGui::InputText(fieldId.c_str(), &str[0], 200);
				*p = std::string(str.c_str());
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
			case mathVector_Type:
			{
				if (it->isIntVector)
				{
					auto *data = reinterpret_cast<int *>(it->data);
					switch (it->vectorSize)
					{
					case 2:
						ImGui::InputInt2(fieldId.c_str(), data);
						break;
					case 3:
						ImGui::InputInt3(fieldId.c_str(), data);
						break;
					case 4:
						ImGui::InputInt4(fieldId.c_str(), data);
						break;
					}
				}
				else
				{
					auto *data = reinterpret_cast<float *>(it->data);
					switch (it->vectorSize)
					{
					case 2:
						ImGui::InputFloat2(fieldId.c_str(), data);
						break;
					case 3:
						ImGui::InputFloat3(fieldId.c_str(), data);
						break;
					case 4:
						ImGui::InputFloat4(fieldId.c_str(), data);
						break;
					}
				}
				break;
			}
			case entityRef_Type:
			{
				SerializableScript *script = dynamic_cast<SerializableScript *>(e.get());
				if (script && script->IsVectorPtrField(std::string(it->name)))
				{
					DrawVectorField(script, it->name, fieldId);
					break;
				}
				auto p = reinterpret_cast<std::string *>(it->data);
				DrawEntityRefField(*p, fieldId);
				break;
			}
			case compRef_Type:
			{
				SerializableScript *script = dynamic_cast<SerializableScript *>(e.get());
				if (script && script->IsVectorPtrField(std::string(it->name)))
				{
					DrawVectorField(script, it->name, fieldId);
					break;
				}

				auto p = reinterpret_cast<std::string *>(it->data);
				DrawCompRefField(*p, it->componentTypeHint, fieldId);
				break;
			}
			case texture_Type:
			{
				auto p = reinterpret_cast<std::string *>(it->data);

				// Show filename for display
				std::string displayName = p->empty() ? "None (drop texture here)" : std::filesystem::path(*p).filename().string();

				ImVec4 boxColor = p->empty() ? ImVec4(0.15f, 0.18f, 0.25f, 1.0f)
											 : ImVec4(0.1f, 0.25f, 0.1f, 1.0f);

				ImGui::PushStyleColor(ImGuiCol_Button, boxColor);
				ImGui::Button(displayName.c_str(), ImVec2(-1, 24));
				ImGui::PopStyleColor();

				// Drop target
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload *payload =
							ImGui::AcceptDragDropPayload("ASSET_PATH"))
					{
						std::string droppedPath =
							static_cast<const char *>(payload->Data);

						std::string ext =std::filesystem::path(droppedPath).extension().string();

						*p = droppedPath; // store the path directly

						// Sync back to Texture object via SerializableScript
						if (auto *script = dynamic_cast<SerializableScript *>(e.get()))
						{
							// Update textureFields if they exist
							std::string fieldName(it->name);
							if (script->textureFields.count(fieldName))
								script->textureFields[fieldName]->SetPath(droppedPath);

							script->NotifyFieldChanged(fieldName);
						}
					}
					ImGui::EndDragDropTarget();
				}

				// Right click to clear
				if (ImGui::BeginPopupContextItem(("##ctx" + fieldId).c_str()))
				{
					if (ImGui::MenuItem("Clear"))
					{
						p->clear();
						if (auto *script = dynamic_cast<SerializableScript *>(e.get()))
						{
							std::string fieldName(it->name);
							if (script->textureFields.count(fieldName))
								script->textureFields[fieldName]->Clear();
							script->NotifyFieldChanged(fieldName);
						}
					}
					ImGui::EndPopup();
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
		if (availableComponents.empty())
		{
			SaveAvailableComponents();
		}

		ImGui::BeginChild("Add Component");
		for (auto &comp : availableComponents)
		{
			if (ImGui::Button(comp.c_str()))
			{
				AddComponentByName(comp);
				addingNewComp = false;
			}
		}
		ImGui::EndChild();
	}
}

// Component factory - create and add component by name
void Entity::AddComponentByName(const std::string &componentName)
{
	if (!ComponentRegistry::get().AddByName(this, componentName))
	{
		LOG_ERROR("Failed to add component (unknown type, duplicate single-instance, or unmet requirements): ", componentName.c_str());
	}
}

// Static metadata map defining which components allow multiple instances
bool Entity::ComponentAllowsMultiple(const std::string &componentName)
{
	return ComponentRegistry::get().IsMultiInstance(componentName);
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
			ser.guiD = c->GetGUID();
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
					case mathVector_Type:
					{
						auto *data = reinterpret_cast<float *>(var.data);
						std::string packed;
						for (int i = 0; i < var.vectorSize; i++)
						{
							if (i > 0)
								packed += "|";
							packed += std::to_string(data[i]);
						}
						LOG_DEBUG("  Vector packed: ", packed.c_str());
						ser.fields.stringFields[var.name] = packed;
						break;
					}
					case texture_Type:
						ser.fields.stringFields[var.name] = *reinterpret_cast<std::string *>(var.data);
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

void Entity::DrawVectorField(SerializableScript *script,
							 const char *fieldName,
							 const std::string &fieldId)
{
	std::string name(fieldName);
	bool isPtrField = script->IsVectorPtrField(name);
	std::string &packedStr = script->vectorStrings[name];

	// Find field type and type hint
	int fieldType = char_Type;
	std::string typeHint = "";
	for (auto &var : *script->GetSerializedFields())
	{
		if (std::string(var.name) == name)
		{
			typeHint = var.componentTypeHint;
			break;
		}
	}

	if (script->vectorFields.count(name))
		fieldType = script->vectorFields[name].elementType;
	else if (script->vectorPtrFields.count(name))
		fieldType = script->vectorPtrFields[name].elementType;
	// Parse current elements
	std::vector<std::string> elements;
	if (!packedStr.empty())
	{
		std::stringstream ss(packedStr);
		std::string token;
		while (std::getline(ss, token, ';'))
			elements.push_back(token);
	}

	int removeIdx = -1;

	for (int i = 0; i < (int)elements.size(); i++)
	{
		std::string elemId = fieldId + "_" + std::to_string(i);

		if (isPtrField)
		{
			float childHeight = ImGui::GetFrameHeightWithSpacing() + 8.f;
			ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.12f, 0.12f, 0.12f, 1.0f));
			ImGui::BeginChild((elemId + "_child").c_str(), ImVec2(0, childHeight), true);

			ImGui::Text("%d.", i + 1);
			ImGui::SameLine();

			if (fieldType == entityRef_Type)
				DrawEntityRefField(elements[i], elemId);
			else if (fieldType == compRef_Type)
				DrawCompRefField(elements[i], typeHint, elemId);

			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
			if (ImGui::Button(("X##remove" + elemId).c_str(), ImVec2(30, 0)))
				removeIdx = i;
			ImGui::PopStyleColor();

			ImGui::EndChild();
			ImGui::PopStyleColor();
		}
		else
		{

			ImGui::Text("%d.", i + 1);
			ImGui::SameLine();
			// Primitive - draw inline using DrawVectorElement
			elements[i] = DrawVectorElement(elements[i], fieldType, elemId);
			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
			if (ImGui::Button(("X##remove" + elemId).c_str(), ImVec2(30, 0)))
				removeIdx = i;
			ImGui::PopStyleColor();
		}
	}

	// Remove element
	if (removeIdx >= 0)
		elements.erase(elements.begin() + removeIdx);

	// Add button
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.4f, 0.1f, 1.0f));
	if (ImGui::Button(("+ Add##add" + fieldId).c_str(), ImVec2(-1, 0)))
		elements.push_back(DefaultValue(fieldType));
	ImGui::PopStyleColor();

	// Rebuild packed string
	std::string newPacked;
	for (size_t i = 0; i < elements.size(); i++)
	{
		if (i > 0)
			newPacked += ";";
		newPacked += elements[i]; // null stays as "null"
	}
	packedStr = newPacked;

	// Sync back to primitive vector
	if (script->vectorFields.count(name))
		script->vectorFields[name].deserialize(packedStr);
}

std::string Entity::DrawVectorElement(const std::string &current,
									  int fieldType,
									  const std::string &elemId)
{
	switch (fieldType)
	{
	case int_Type:
	{
		int val = 0;
		try
		{
			if (!current.empty())
				val = std::stoi(current);
		}
		catch (...)
		{
		}
		ImGui::SetNextItemWidth(-40);
		if (ImGui::InputInt(("##" + elemId).c_str(), &val))
			return std::to_string(val);
		return current;
	}
	case float_Type:
	{
		float val = 0.f;
		try
		{
			if (!current.empty())
				val = std::stof(current);
		}
		catch (...)
		{
		}
		ImGui::SetNextItemWidth(-40);
		if (ImGui::InputFloat(("##" + elemId).c_str(), &val))
			return std::to_string(val);
		return current;
	}
	case bool_Type:
	{
		bool val = current == "1";
		if (ImGui::Checkbox(("##" + elemId).c_str(), &val))
			return val ? "1" : "0";
		return current;
	}
	case entityRef_Type:
	case compRef_Type:
		return current;
	case char_Type:
	default:
	{
		std::string str = current;
		str.resize(200, '\0');
		ImGui::SetNextItemWidth(-40);
		if (ImGui::InputText(("##" + elemId).c_str(), &str[0], 200))
			return std::string(str.c_str());
		return current;
	}
	}
}

std::string Entity::DefaultValue(int fieldType)
{
	switch (fieldType)
	{
	case int_Type:
		return "0";
	case float_Type:
		return "0.0";
	case bool_Type:
		return "0";
	case char_Type:
		return "";
	case entityRef_Type:
		return "null"; // null entity
	case compRef_Type:
		return "null"; // null component
	case texture_Type:
		return "null"; // null texture
	default:
		return "";
	}
}

void Entity::DrawEntityRefField(std::string &guidStorage, const std::string &fieldId)
{
	bool isEmpty = guidStorage.empty() || guidStorage == "null";

	Entity *resolved = nullptr;
	if (!isEmpty)
	{
		for (auto &ent : Engine::get().GetManager()->GetEntities())
		{
			if (ent->GetGUID() == guidStorage)
			{
				resolved = ent.get();
				break;
			}
		}
	}

	std::string displayText = resolved ? "-> " + resolved->GetName() : !isEmpty ? "NOT FOUND"
																				: "Drop entity here...";

	ImVec4 boxColor = resolved ? ImVec4(0.1f, 0.25f, 0.1f, 1.0f) : !guidStorage.empty() ? ImVec4(0.35f, 0.1f, 0.1f, 1.0f)
																						: ImVec4(0.15f, 0.18f, 0.25f, 1.0f);

	ImGui::PushStyleColor(ImGuiCol_Button, boxColor);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
						  ImVec4(boxColor.x + 0.05f, boxColor.y + 0.05f,
								 boxColor.z + 0.1f, 1.0f));
	ImGui::Button((displayText + "##" + fieldId).c_str(), ImVec2(-40, 0));
	ImGui::PopStyleColor(2);

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("ENTITY"))
		{
			Entity *dropped = *(Entity **)payload->Data;
			if (dropped)
				guidStorage = dropped->GetGUID();
		}
		ImGui::EndDragDropTarget();
	}
}

void Entity::DrawCompRefField(std::string &packedStorage,
							  const std::string &typeHint,
							  const std::string &fieldId)
{
	bool isEmpty = packedStorage.empty() || packedStorage == "null";

	std::string entityGUID, compGUID;
	if (!isEmpty)
	{
		auto pipe = packedStorage.find('|');
		if (pipe != std::string::npos)
		{
			entityGUID = packedStorage.substr(0, pipe);
			compGUID = packedStorage.substr(pipe + 1);
		}
	}

	Entity *resolved = nullptr;
	if (!entityGUID.empty())
	{
		for (auto &ent : Engine::get().GetManager()->GetEntities())
		{
			if (ent->GetGUID() == entityGUID)
			{
				resolved = ent.get();
				break;
			}
		}
	}

	std::string displayText = "Drop component here...";
	if (resolved)
	{
		std::string compName = typeHint;
		for (auto &comp : resolved->GetComponents())
		{
			if (comp->GetGUID() == compGUID)
			{
				compName = typeid(*comp).name();
				compName = std::regex_replace(compName, std::regex("class "), "");
				break;
			}
		}
		displayText = compName + " (" + resolved->GetName() + ")";
	}
	else if (!isEmpty && !entityGUID.empty())
		displayText = "NOT FOUND";

	ImVec4 boxColor = resolved ? ImVec4(0.1f, 0.25f, 0.1f, 1.0f) : (!isEmpty && !entityGUID.empty()) ? ImVec4(0.35f, 0.1f, 0.1f, 1.0f)
																									 : ImVec4(0.15f, 0.18f, 0.25f, 1.0f);

	ImGui::PushStyleColor(ImGuiCol_Button, boxColor);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
						  ImVec4(boxColor.x + 0.05f, boxColor.y + 0.05f,
								 boxColor.z + 0.1f, 1.0f));
	ImGui::Button((displayText + "##" + fieldId).c_str(), ImVec2(-40, 0));
	ImGui::PopStyleColor(2);

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("ENTITY"))
		{
			Entity *dropped = *(Entity **)payload->Data;
			if (dropped)
			{
				for (auto &comp : dropped->GetComponents())
				{
					std::string compTypeName(typeid(*comp).name());
					compTypeName = std::regex_replace(compTypeName,
													  std::regex("class "), "");
					if (compTypeName == "Transform" ||
						compTypeName == "EditorSprite")
						continue;
					if (typeHint.empty() || compTypeName == typeHint)
					{
						packedStorage = dropped->GetGUID() + "|" + comp->GetGUID();
						break;
					}
				}
			}
		}
		ImGui::EndDragDropTarget();
	}
}

void Entity::DrawTextureField(std::string &value, const std::string &fieldId)
{
}