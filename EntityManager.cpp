#include "EntityManager.h"
#include "Entity.h"
#include "Rigidbody.h"
#include "CollisionSystem.h"
#include "BoxCollider2D.h"

void EntityManager::draw()
{
	for (auto& entity : entities)
	{
		entity->Draw();
	}
}

void EntityManager::update()
{
	for (auto& entity : entities)
	{
		entity->Update();
	}
}

void EntityManager::Collisions()
{
	for (auto& entity : entities)
	{
		if (!entity->HasComponent<BoxCollider2D>())
			continue;

		for (auto& other : entities)
		{
			if (entity == other)
				continue;

			if (!other->HasComponent<BoxCollider2D>())
				continue;

			auto coll1 = entity->GetComponent<BoxCollider2D>();
			auto coll2 = other->GetComponent<BoxCollider2D>();

			if (CollisionSystem::get().AABB(coll1.GetRect(), coll2.GetRect()))
			{
				//std::cout << coll1.GetCollisionTag() << " , " << coll2.GetCollisionTag() << std::endl;
				if (CollisionSystem::get().ActiveCollision(coll1.GetCollisionTag(), coll2.GetCollisionTag()))
				{
					coll1.entity->OnTriggerStay(coll2);
					coll2.entity->OnTriggerStay(coll1);
					return;
				}

				CollisionSystem::get().SetActive(coll1.GetCollisionTag(), coll2.GetCollisionTag());
				if (coll1.isTrigger)
				{
					coll1.entity->OnTriggerEnter(coll2);
				}
				else
				{
					coll1.entity->OnCollisionEnter(coll2);
				}
				if (coll2.isTrigger)
				{
					coll2.entity->OnTriggerEnter(coll1);
				}
				else
				{
					coll2.entity->OnCollisionEnter(coll1);
				}
			}
			else
			{
				if (CollisionSystem::get().ActiveCollision(coll1.GetCollisionTag(), coll2.GetCollisionTag()))
				{
					if (coll1.isTrigger)
					{
						coll1.entity->OnTriggerExit(coll2);
					}
					else
					{
						coll1.entity->OnCollisionExit(coll2);
					}
					if (coll2.isTrigger)
					{
						coll2.entity->OnTriggerExit(coll1);
					}
					else
					{
						coll2.entity->OnCollisionExit(coll1);
					}
					CollisionSystem::get().SetInactive(coll1.GetCollisionTag(), coll2.GetCollisionTag());
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

	entities.emplace_back(std::move(uniquePtr));
}

void EntityManager::eraseEntity(Entity* ent)
{
}

Entity* EntityManager::cloneEntity(Entity* ent)
{
	return nullptr;
}

