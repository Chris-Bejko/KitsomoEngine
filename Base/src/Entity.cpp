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

Entity::~Entity()
{
	if (parent)
	{
		parent->RemoveChild(this);
		parent = nullptr;
	}

	for (Entity *child : children)
	{
		if (child)
		{
			child->parent = nullptr;
			if (child->transform)
				child->transform->SetParent(nullptr);
		}
	}
	children.clear();

	if (transform)
	{
		transform->SetParent(nullptr);
	}

	if (Engine::get().GetManager())
	{
		if (Engine::get().GetManager()->GetSelectedEntity() == this)
		{
			Engine::get().GetManager()->SetSelectedEntity(nullptr);
		}
		if (Engine::get().GetManager()->GetDragHoveredEntity() == this)
		{
			Engine::get().GetManager()->SetDragHoveredEntity(nullptr);
		}
		Engine::get().GetManager()->RemoveCollisionPairsForEntity(this);
	}
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
		if(++depth > 100)
			return false;
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
