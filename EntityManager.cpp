#include "EntityManager.h"
#include "Collision/CollisionSystem.h"
#include "Components/BoxCollider.h"
#include "Engine.h"
#include "Components/Sprite.h"
#include "Logger.h"
#include "Components/Rigidbody.h"
void EntityManager::draw()
{
	std::map<int, std::vector<std::unique_ptr<Entity> *>> renderBuckets;
	std::vector<std::unique_ptr<Entity> *> noRenderOrderEntities;

	// Group entities by RenderOrder
	for (auto &entity : entities)
	{
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
		entity->UpdateEngine(dt);
	}
	ValidateRemoved();
}
void EntityManager::update(float dt)
{

	ValidateAdded();
	for (auto &entity : entities)
	{
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
									  return !entity->IsActive();
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

void EntityManager::DisplayEntities()
{
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 6));

	for (auto &e : entities)
	{
		std::string name = e->GetName().c_str();
		bool isSelected = (selectedEntity == e.get());

		if (isSelected)
			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.4f, 0.7f, 0.6f));

		if (ImGui::Checkbox(name.c_str(), &e->displayComponents))
		{
			for (auto &a : entities)
			{
				if (a.get() == e.get())
					continue;
				a->displayComponents = false;
			}
			selectedEntity = e->displayComponents ? e.get() : nullptr;
		}

		if (isSelected)
			ImGui::PopStyleColor();
	}

	ImGui::PopStyleVar();
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
	entities.clear();
}

void EntityManager::Collisions()
{
	if (Engine::get().isEngine)
		return;

	for (auto &entity : entities)
	{
		Collider *coll1 = entity->GetComponentOfType<Collider>();
		if (!coll1)
			continue;

		for (auto &other : entities)
		{
			if (entity == other)
				continue;

			Collider *coll2 = other->GetComponentOfType<Collider>();
			if (!coll2)
				continue;

			bool hit = CollisionSystem::get().CheckCollision(coll1, coll2);
			if (hit)
			{
				bool alreadyActive = CollisionSystem::get().ActiveCollision(coll1, coll2);
				LOG_DEBUG("Hit! active: ", alreadyActive,
						  " tag-: ", coll1->GetCollisionTag().c_str(),
						  "  tag--: ", coll2->GetCollisionTag().c_str());
				if (CollisionSystem::get().ActiveCollision(coll1, coll2))
				{
					coll1->entity->OnTriggerStay(*coll2);
					coll2->entity->OnTriggerStay(*coll1);
					continue;
				}

				CollisionSystem::get().SetActive(coll1, coll2);

				if (coll1->IsTrigger())
					coll1->entity->OnTriggerEnter(*coll2);
				else
					coll1->entity->OnCollisionEnter(*coll2);

				if (coll2->IsTrigger())
					coll2->entity->OnTriggerEnter(*coll1);
				else
					coll2->entity->OnCollisionEnter(*coll1);

				    if (!coll1->IsTrigger() && !coll2->IsTrigger())
        				CollisionSystem::get().ResolveCollision(coll1, coll2);
			}
			else
			{
				if (CollisionSystem::get().ActiveCollision(coll1, coll2))
				{
					if (coll1->IsTrigger())
						coll1->entity->OnTriggerExit(*coll2);
					else
						coll1->entity->OnCollisionExit(*coll2);

					if (coll2->IsTrigger())
						coll2->entity->OnTriggerExit(*coll1);
					else
						coll2->entity->OnCollisionExit(*coll1);

					CollisionSystem::get().SetInactive(coll1, coll2);
				}
			}
		}
	}
}
void EntityManager::refresh()
{
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

// EntityManager.cpp
std::string EntityManager::GetUniqueName(const std::string &baseName)
{
	std::string cleanBase = baseName.c_str(); // strip null chars and garbage

	bool baseTaken = false;
	for (auto &e : entities)
		if (e->GetName() == cleanBase)
			baseTaken = true;

	if (!baseTaken)
		return cleanBase;

	int counter = 1;
	while (true)
	{
		std::string candidate = cleanBase + " (" + std::to_string(counter) + ")";
		bool taken = false;
		for (auto &e : entities)
			if (e->GetName() == candidate)
				taken = true;

		if (!taken)
			return candidate;
		counter++;
	}
}