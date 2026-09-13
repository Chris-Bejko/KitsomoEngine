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
	std::map<int, std::vector<Entity *>> renderBuckets;
	std::vector<Entity *> noRenderOrderEntities;

	// Group entities by RenderOrder
	for (size_t i = 0; i < entities.size(); ++i)
	{
		Entity *entity = entities[i].get();
		if (!entity || entity->IsPendingDestroy() || !entity->IsActiveInHierarchy())
			continue;

		if (entity->HasComponent<Sprite>())
		{
			auto comp = &entity->GetComponent<Sprite>();
			renderBuckets[comp->RenderOrder()].push_back(entity);
		}
		else
		{
			noRenderOrderEntities.push_back(entity);
		}
	}

	// Draw entities by RenderOrder
	for (auto &[order, bucket] : renderBuckets)
	{
		for (auto *entityPtr : bucket)
		{
			if (entityPtr && !entityPtr->IsPendingDestroy())
				entityPtr->Draw();
		}
	}

	// Draw entities with no RenderOrder last
	for (auto *entityPtr : noRenderOrderEntities)
	{
		if (entityPtr && !entityPtr->IsPendingDestroy())
			entityPtr->Draw();
	}
}

void EntityManager::updateEngine(float dt)
{
	ValidateAdded();
	for (size_t i = 0; i < entities.size(); ++i)
	{
		if (!entities[i] || entities[i]->IsPendingDestroy())
			continue;
		entities[i]->UpdateEngine(dt);
	}
	ValidateRemoved();
}
void EntityManager::update(float dt)
{
	ValidateAdded();
	for (size_t i = 0; i < entities.size(); ++i)
	{
		if (!entities[i] || entities[i]->IsPendingDestroy())
			continue;
		if (!entities[i]->IsActiveInHierarchy())
			continue;
		entities[i]->Update(dt);
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
									  return !entity || entity->IsPendingDestroy();
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

void EntityManager::ClearInspector()
{
	for (auto &e : entities)
	{
		e->displayComponents = false;
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
	for (size_t i = 0; i < entities.size(); ++i)
	{
		Entity *entity = entities[i].get();
		if (!entity || entity->IsPendingDestroy() || !entity->IsActiveInHierarchy())
			continue;

		Collider *coll1 = entity->GetComponentOfType<Collider>();
		if (!coll1)
			continue;

		for (size_t j = 0; j < entities.size(); ++j)
		{
			if (i == j)
				continue;

			Entity *other = entities[j].get();
			if (!other || other->IsPendingDestroy() || !other->IsActiveInHierarchy())
				continue;

			if (entity->GetGUID() == other->GetGUID())
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
					if (!coll1->entity->IsPendingDestroy() && !coll2->entity->IsPendingDestroy())
					{
						coll1->entity->OnTriggerStay(*coll2);
					}
					if (!coll2->entity->IsPendingDestroy() && !coll1->entity->IsPendingDestroy())
					{
						coll2->entity->OnTriggerStay(*coll1);
					}
				}
				else
				{
					// New collision - call OnTriggerEnter/OnCollision (first time)
					LOG_DEBUG("OnTriggerEnter - collision started");
					activeCollisionPairs.insert({coll1, coll2});

					if (!coll1->entity->IsPendingDestroy() && !coll2->entity->IsPendingDestroy())
					{
						if (coll1->IsTrigger())
							coll1->entity->OnTriggerEnter(*coll2);
						else
							coll1->entity->OnCollisionEnter(*coll2);
					}

					if (!coll2->entity->IsPendingDestroy() && !coll1->entity->IsPendingDestroy())
					{
						if (coll2->IsTrigger())
							coll2->entity->OnTriggerEnter(*coll1);
						else
							coll2->entity->OnCollisionEnter(*coll1);
					}

					// Physics collision resolution
					if (!coll1->entity->IsPendingDestroy() && !coll2->entity->IsPendingDestroy() && !coll1->IsTrigger() && !coll2->IsTrigger())
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
			if (!coll1 || !coll2 || !coll1->entity || !coll2->entity)
			{
				toRemove.insert(pair);
				continue;
			}

			if (coll1->entity->IsPendingDestroy() || coll2->entity->IsPendingDestroy() ||
				!coll1->entity->IsActiveInHierarchy() || !coll2->entity->IsActiveInHierarchy())
			{
				toRemove.insert(pair);
				continue;
			}

			if (coll1->IsTrigger())
				coll1->entity->OnTriggerExit(*coll2);
			else
				coll1->entity->OnCollisionExit(*coll2);

			if (!coll2->entity->IsPendingDestroy())
			{
				if (coll2->IsTrigger())
					coll2->entity->OnTriggerExit(*coll1);
				else
					coll2->entity->OnCollisionExit(*coll1);
			}

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
		if (!pair.first || !pair.second || !pair.first->entity || !pair.second->entity ||
			pair.first->entity == e || pair.second->entity == e)
		{
			toRemove.insert(pair);
		}
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
						   return !e || e->GetGUID() == guid;
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
		e->ValidateAddedComponents();
	}
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
	if (ent)
	{
		ent->Destroy();
	}
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
		ser.components = e->GetSerializedComponents();
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
