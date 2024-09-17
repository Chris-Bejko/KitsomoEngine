#pragma once

#include <vector>
#include <memory>
#include "Entity.h"

class EntityManager
{
public:
    EntityManager() = default;
    ~EntityManager() = default;

    void draw();
    void update();
    void refresh();
    void Collisions();


    void addEntity(Entity* ent);
    void eraseEntity(Entity* ent);

    Entity* cloneEntity(Entity* ent);
private:
    std::vector<std::unique_ptr<Entity>> entities;
};