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

	std::vector<std::vector<std::string>> GetActiveCollisions()
	{
		return activeCollisions;
	}

	void AddColliders(std::vector<std::string>& Colliders)
	{
		activeCollisions.push_back(Colliders);
	}

	void RemoveActiveCollision(std::vector<std::string> it)
	{
		activeCollisions.erase(std::remove(activeCollisions.begin(), activeCollisions.end(), it), activeCollisions.end());
	}

private:
	std::vector<std::unique_ptr<Entity>> entities;

	std::vector<std::vector<std::string>> activeCollisions;
};