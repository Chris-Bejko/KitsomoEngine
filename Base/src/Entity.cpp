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

std::vector<SerializedComponent> Entity::GetSerializedComponents()
{
	std::vector<SerializedComponent> result;

	for (auto &component : components)
	{
		if (!component)
			continue;

		const auto &fields = component->GetSerializedFields();

		if (fields.empty())
			continue;

		std::string componentName = typeid(*component).name();
		componentName = std::regex_replace(componentName, std::regex("class "), "");

		SerializedComponent serialized(componentName, component->GetGUID());

		for (const auto &field : fields)
		{
			if (!field)
				continue;

			serialized.AddSerializedField(field->GetName(), field->Serialize());
		}

		result.push_back(std::move(serialized));
	}

	return result;
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
		auto &fields = e->GetSerializedFields();

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
		int uniqueFieldId = 0;
		for (auto &field : fields)
		{
			auto *script = dynamic_cast<SerializableScript *>(e.get());
			field->Draw({this, script, uniqueId + "##" + std::to_string(uniqueFieldId)});
			uniqueFieldId++;
		}
		// Fields
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
