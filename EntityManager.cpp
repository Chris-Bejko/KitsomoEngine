#include "EntityManager.h"
#include "CollisionSystem.h"
#include "Components/BoxCollider.h"
#include "Engine.h"

void EntityManager::draw()
{
	for (auto& entity : entities)
	{
		entity->Draw();
	}
}

void EntityManager::update(float dt)
{
	while (to_add.size() > 0)
	{
		entities.push_back(std::move(to_add.back()));

		to_add.pop_back();
	}
	//to_add.clear();
	for (auto& entity : entities)
	{
		entity->Update(dt);
	}

	entities.erase(std::remove_if(entities.begin(), entities.end(),
		[](const std::unique_ptr<Entity>& entity) {
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

void EntityManager::AddColliders(std::vector<std::string>& Colliders)
{
	activeCollisions.push_back(Colliders);
}

void EntityManager::RemoveActiveCollision(std::vector<std::string> it)
{
	activeCollisions.erase(std::remove(activeCollisions.begin(), activeCollisions.end(), it), activeCollisions.end());
}

void EntityManager::DisplayEntities()
{
	for (auto& e : entities)
	{
		auto temp = e->GetName();
		if (ImGui::Checkbox(temp.c_str(), &e->displayComponents))
		{
			for (auto& a : entities)
			{
				if (a->GetName() == e->GetName())
				{
					continue;
				}
				a->displayComponents = false;
			}
		}
	}
}

void EntityManager::DisplayComponents()
{
	for (auto& e : entities)
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
	for (auto& entity : entities)
	{
		if (!entity->HasComponent<BoxCollider>())
			continue;

		for (auto& other : entities)
		{
			if (entity == other)
				continue;

			if (!other->HasComponent<BoxCollider>())
				continue;

			auto  coll1 = &entity->GetComponent<BoxCollider>();
			auto  coll2 = &other->GetComponent<BoxCollider>();
			if (!&coll1 || !&coll2)
				return;

			if (CollisionSystem::get().AABB(coll1->GetRect(), coll2->GetRect()))
			{

				////std::cout << coll1.GetCollisionTag() << " , " << coll2.GetCollisionTag() << std::endl;
				if (CollisionSystem::get().ActiveCollision(coll1->GetCollisionTag(), coll2->GetCollisionTag()))
				{

					coll1->entity->OnTriggerStay(*coll2);
					coll2->entity->OnTriggerStay(*coll1);
					return;
				}

				CollisionSystem::get().SetActive(coll1->GetCollisionTag(), coll2->GetCollisionTag());
				if (coll1->IsTrigger())
				{
					coll1->entity->OnTriggerEnter(*coll2);
				}
				else
				{
					coll1->entity->OnCollisionEnter(*coll2);
				}
				if (coll2->IsTrigger())
				{
					coll2->entity->OnTriggerEnter(*coll1);
				}
				else
				{
					coll2->entity->OnCollisionEnter(*coll1);
				}
			}
			else
			{
				if (CollisionSystem::get().ActiveCollision(coll1->GetCollisionTag(), coll2->GetCollisionTag()))
				{
					if (coll1->IsTrigger())
					{
						coll1->entity->OnTriggerExit(*coll2);
					}
					else
					{
						coll1->entity->OnCollisionExit(*coll2);
					}
					if (coll2->IsTrigger())
					{
						coll2->entity->OnTriggerExit(*coll1);
					}
					else
					{
						coll2->entity->OnCollisionExit(*coll1);
					}
					CollisionSystem::get().SetInactive(coll1->GetCollisionTag(), coll2->GetCollisionTag());
				}
			}
		}
	}
}
void EntityManager::refresh()
{
}

void EntityManager::addEntity(Entity* ent)
{
	std::unique_ptr<Entity> uniquePtr{ ent };
	to_add.push_back(std::move(uniquePtr));
}

void EntityManager::eraseEntity(Entity* ent)
{
	ent->Destroy();
}

Entity* EntityManager::cloneEntity(Entity* ent)
{
	return nullptr;
}

std::vector<SerializableEntity> EntityManager::SerializeEntities()
{
	std::vector<SerializableEntity> entitiesSerialized;

	for (auto& e : entities)
	{
		SerializableEntity ser;
		ser.entityName = e->GetName();
		ser.components = e->GetAllComponentVariables();
		entitiesSerialized.push_back(ser);
	}

	return entitiesSerialized;
}