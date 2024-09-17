#include "EntityManager.h"
#include "Entity.h"
#include "BoxCollider2D.h"
#include "Rigidbody.h"
#include "CollisionSystem.h"

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
    for (auto &entity : entities)
    {
        if (!entity->HasComponent<BoxCollider2D>())
            continue;

        for (auto &other : entities)
        {
            if (entity == other)
                continue;

            if (!other->HasComponent<BoxCollider2D>())
                continue;

            auto coll1 = entity->GetComponent<BoxCollider2D>();
            auto coll2 = other->GetComponent<BoxCollider2D>();
          
            if (CollisionSystem::get().AABB(coll1.GetRect(), coll2.GetRect()))
            {
                coll1.entity->OnCollisionEnter(coll2);
                coll2.entity->OnCollisionEnter(coll1);
                std::cout << "Collision Detected between " << coll1.GetCollisionTag() << "," << coll2.GetCollisionTag() << std::endl;
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
