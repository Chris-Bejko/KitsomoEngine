#include "EntityManager.h"
#include "Collision/CollisionSystem.h"
#include "BoxCollider.h"
#include "Engine.h"
#include "Sprite.h"
#include "Logger.h"
#include "Rigidbody.h"
#include "GizmoSystem.h"

void EntityManager::draw()
{
	std::map<int, std::vector<std::unique_ptr<Entity> *>> renderBuckets;
	std::vector<std::unique_ptr<Entity> *> noRenderOrderEntities;

	// Group entities by RenderOrder
	for (auto &entity : entities)
	{
		if (!entity->IsActiveInHierarchy())
			continue;

		if (entity->HasComponent<Sprite>())
		{
			auto comp = &entity->GetComponent<Sprite>();
			renderBuckets[comp->RenderOrder()].emplace_back(&entity); // Use a pointer to the unique_ptr
		}
		else
		{
			noRenderOrderEntities.emplace_back(&entity); // Handle entities without Sprite component
		}
	}

	// Draw entities by RenderOrder
	for (auto &[order, bucket] : renderBuckets)
	{
		for (auto *entityPtr : bucket)
		{
			(*entityPtr)->Draw(); // Dereference to access the underlying object
		}
	}

	// Draw entities with no RenderOrder last
	for (auto *entityPtr : noRenderOrderEntities)
	{
		(*entityPtr)->Draw(); // Dereference to access the underlying object
	}
}

void EntityManager::updateEngine(float dt)
{
	ValidateAdded();
	for (auto &entity : entities)
	{
		if (!entity)
			continue;
		entity->UpdateEngine(dt);
	}
	ValidateRemoved();
}
void EntityManager::update(float dt)
{
	ValidateAdded();
	for (auto &entity : entities)
	{
		if (!entity->IsActiveInHierarchy())
			continue;
		entity->Update(dt);
	}
	ValidateRemoved();
}

void EntityManager::ValidateAdded()
{
	while (to_add.size() > 0)
	{
		entities.push_back(std::move(to_add.back()));
		to_add.pop_back();
	}
}

void EntityManager::ValidateRemoved()
{
	entities.erase(std::remove_if(entities.begin(), entities.end(),
								  [](const std::unique_ptr<Entity> &entity)
								  {
									  return entity->IsPendingDestroy();
								  }),
				   entities.end());
}
size_t EntityManager::GetTotalEntities()
{
	return entities.size();
}

std::vector<std::vector<std::string>> EntityManager::GetActiveCollisions()
{
	return activeCollisions;
}

void EntityManager::AddColliders(std::vector<std::string> &Colliders)
{
	activeCollisions.push_back(Colliders);
}

void EntityManager::RemoveActiveCollision(std::vector<std::string> it)
{
	activeCollisions.erase(std::remove(activeCollisions.begin(), activeCollisions.end(), it), activeCollisions.end());
}

void EntityManager::SetSelectedEntity(Entity *entity)
{
	selectedEntity = entity;
	GizmoSystem::get().SetSelectedEntity(entity);
}
void EntityManager::DisplayEntities()
{
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 6));

	if (ImGui::GetDragDropPayload() == nullptr)
		dragHoveredEntity = nullptr;

	for (auto &e : entities)
	{
		if (!e || e->IsPendingDestroy())
			continue;
		if (e->HasParent())
			continue;
		DisplayEntityNode(e.get());
	}

	ImGui::PopStyleVar();
}

void EntityManager::DisplayEntityNode(Entity *e)
{

	if (!e || e->IsPendingDestroy())
		return;
	std::string name = e->GetName().c_str();
	bool isSelected = (selectedEntity == e);
	bool hasChildren = !e->GetChildren().empty();

	if (isSelected)
		ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.4f, 0.7f, 0.6f));

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow |
							   ImGuiTreeNodeFlags_SpanAvailWidth;
	if (!hasChildren)
		flags |= ImGuiTreeNodeFlags_Leaf;
	if (isSelected)
		flags |= ImGuiTreeNodeFlags_Selected;

	bool opened = ImGui::TreeNodeEx(name.c_str(), flags);

	if (ImGui::IsItemClicked())
	{
		for (auto &a : entities)
			a->displayComponents = false;

		e->displayComponents = true;
		selectedEntity = e;
		GizmoSystem::get().SetSelectedEntity(e);

		float currentTime = ImGui::GetTime();
		if (lastClickedEntity == e && (currentTime - lastClickTime) < 0.3f)
		{
			Engine::get().FocusOnEntity(e);
			e->displayComponents = true;
		}
		lastClickedEntity = e;
		lastClickTime = currentTime;
	}

	// Track hover for preview
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) && ImGui::GetDragDropPayload() != nullptr)
		dragHoveredEntity = e;

	if (ImGui::BeginDragDropSource())
	{
		Entity *ptr = e;
		ImGui::SetDragDropPayload("ENTITY", &ptr, sizeof(Entity *));
		ImGui::Text("%s", name.c_str());
		ImGui::EndDragDropSource();
	}

	// Drop target - reparent only
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("ENTITY"))
		{
			Entity *dragged = *(Entity **)payload->Data;
			if (dragged != e)
				dragged->SetParent(e);
		}
		ImGui::EndDragDropTarget();
	}

	if (isSelected)
		ImGui::PopStyleColor();

	if (opened)
	{
		for (auto *child : e->GetChildren())
			DisplayEntityNode(child);
		ImGui::TreePop();
	}
}

void EntityManager::ClearInspector()
{
	for (auto &e : entities)
	{
		e->displayComponents = false;
	}
}

void EntityManager::DisplayComponents()
{
	for (auto &e : entities)
	{
		e->DisplayComponents();
	}
}

void EntityManager::DestroyAllEntities()
{
	for (auto &e : entities)
	{
		e->transform->ClearHierarchy();
		e->SetParent(nullptr);
		e->GetChildren().clear();
	}
	for (auto &e : entities)
	{
		e->SetParent(nullptr);
		e->transform->SetParent(nullptr);
		e->GetChildren().clear();
	}
	entities.clear();
	to_add.clear();
}

void EntityManager::Collisions()
{
	if (Engine::get().isEngine)
		return;

	// Track which pairs collided this frame
	std::set<std::pair<Collider *, Collider *>> collidingThisFrame;

	// Check all collisions
	for (auto &entity : entities)
	{
		Collider *coll1 = entity->GetComponentOfType<Collider>();
		if (!entity->IsActiveInHierarchy())
			continue;

		if (!coll1)
			continue;

		for (auto &other : entities)
		{
			if (entity->GetGUID() == other->GetGUID())
				continue;
			if (!other->IsActiveInHierarchy())
				continue;

			Collider *coll2 = other->GetComponentOfType<Collider>();
			if (!coll2)
				continue;

			// Only process each pair once (in one direction)
			if (collidingThisFrame.count({coll1, coll2}) || collidingThisFrame.count({coll2, coll1}))
				continue;

			bool hit = CollisionSystem::get().CheckCollision(coll1, coll2);

			if (hit)
			{
				// Mark as colliding this frame
				collidingThisFrame.insert({coll1, coll2});

				// Check if this pair was already active last frame
				bool wasActive = activeCollisionPairs.count({coll1, coll2}) || activeCollisionPairs.count({coll2, coll1});

				if (wasActive)
				{
					// Collision continues - call OnTriggerStay
					LOG_DEBUG("OnTriggerStay - collision continuous");
					coll1->entity->OnTriggerStay(*coll2);
					coll2->entity->OnTriggerStay(*coll1);
				}
				else
				{
					// New collision - call OnTriggerEnter/OnCollision (first time)
					LOG_DEBUG("OnTriggerEnter - collision started");
					activeCollisionPairs.insert({coll1, coll2});

					if (coll1->IsTrigger())
						coll1->entity->OnTriggerEnter(*coll2);
					else
						coll1->entity->OnCollisionEnter(*coll2);

					if (coll2->IsTrigger())
						coll2->entity->OnTriggerEnter(*coll1);
					else
						coll2->entity->OnCollisionEnter(*coll1);

					// Physics collision resolution
					if (!coll1->IsTrigger() && !coll2->IsTrigger())
						CollisionSystem::get().ResolveCollision(coll1, coll2);
				}
			}
		}
	}

	// Check for collisions that ended (were active last frame but not this frame)
	std::set<std::pair<Collider *, Collider *>> toRemove;
	for (auto &pair : activeCollisionPairs)
	{
		if (!collidingThisFrame.count(pair) && !collidingThisFrame.count({pair.second, pair.first}))
		{
			// This collision ended
			Collider *coll1 = pair.first;
			Collider *coll2 = pair.second;

			LOG_DEBUG("OnTriggerExit - collision ended");
			if (!coll1->entity->IsActiveInHierarchy() || !coll2->entity->IsActiveInHierarchy())
				continue;
			if (coll1->IsTrigger())
				coll1->entity->OnTriggerExit(*coll2);
			else
				coll1->entity->OnCollisionExit(*coll2);

			if (coll2->IsTrigger())
				coll2->entity->OnTriggerExit(*coll1);
			else
				coll2->entity->OnCollisionExit(*coll1);

			toRemove.insert(pair);
		}
	}

	// Remove ended collisions
	for (auto &pair : toRemove)
	{
		activeCollisionPairs.erase(pair);
	}
}

void EntityManager::RemoveCollisionPairsForEntity(Entity *e)
{
	std::set<std::pair<Collider *, Collider *>> toRemove;
	for (auto &pair : activeCollisionPairs)
	{
		if (pair.first->entity == e || pair.second->entity == e)
			toRemove.insert(pair);
	}
	for (auto &pair : toRemove)
		activeCollisionPairs.erase(pair);
}
void EntityManager::refresh()
{
}

void EntityManager::RemoveEntityByGUID(const std::string &guid)
{
	entities.erase(
		std::remove_if(entities.begin(), entities.end(),
					   [&guid](const std::unique_ptr<Entity> &e)
					   {
						   return e && e->GetGUID() == guid;
					   }),
		entities.end());
}
bool EntityManager::IsInColliderEditMode()
{
	for (auto &entity : entities)
	{
		Collider *coll = entity->GetComponentOfType<Collider>();
		if (!coll)
			continue;
		if (coll->IsInEditMode())
			return true;
	}
	return false;
}

void EntityManager::Awake()
{
	ValidateAdded();
	for (auto &e : entities)
	{
		e->Awake();
	}
	ValidateRemoved();
}

void EntityManager::addEntity(Entity *ent)
{
	ent->SetName(GetUniqueName(ent->GetName().c_str()));
	std::unique_ptr<Entity> uniquePtr{ent};
	to_add.push_back(std::move(uniquePtr));
}

void EntityManager::eraseEntity(Entity *ent)
{
	ent->Destroy();
}

Entity *EntityManager::cloneEntity(Entity *ent)
{
	return nullptr;
}

std::vector<SerializableEntity> EntityManager::SerializeEntities()
{
	std::vector<SerializableEntity> entitiesSerialized;

	for (auto &e : entities)
	{
		SerializableEntity ser;
		ser.entityName = e->GetName();
		ser.components = e->GetAllComponentVariables();
		entitiesSerialized.push_back(ser);
	}

	return entitiesSerialized;
}

std::string EntityManager::GetUniqueName(const std::string &baseName)
{
	std::string cleanBase = baseName.c_str();

	// Strip existing (N) suffix
	auto pos = cleanBase.rfind(" (");
	if (pos != std::string::npos)
	{
		std::string suffix = cleanBase.substr(pos + 2);
		if (!suffix.empty() && suffix.back() == ')')
		{
			suffix.pop_back();
			bool isNumber = !suffix.empty() &&
							std::all_of(suffix.begin(), suffix.end(), ::isdigit);
			if (isNumber)
				cleanBase = cleanBase.substr(0, pos);
		}
	}

	// Check both entities AND to_add
	auto isTaken = [&](const std::string &name)
	{
		for (auto &e : entities)
			if (e->GetName() == name)
				return true;
		for (auto &e : to_add)
			if (e->GetName() == name)
				return true;
		return false;
	};

	if (!isTaken(cleanBase))
		return cleanBase;

	int counter = 1;
	while (true)
	{
		std::string candidate = cleanBase + " (" + std::to_string(counter) + ")";
		if (!isTaken(candidate))
			return candidate;
		counter++;
	}
}

void EntityManager::DisplayComponentsOf(Entity *e)
{
	if (!e)
		return;
	bool prev = e->displayComponents;
	e->displayComponents = true; // force show
	e->DisplayComponents();
	e->displayComponents = prev; // restore
}