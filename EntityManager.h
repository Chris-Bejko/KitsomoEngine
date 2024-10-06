#pragma once

#include <vector>
#include <memory>
#include "Entity.h"
#include "imgui.h"
#include "imgui-SFML.h"

class SerializableEntity;

class EntityManager
{
public:
	EntityManager() = default;
	~EntityManager() = default;

	void draw();
	void updateEngine(float dt);
	void update(float dt);
	void refresh();
	void Awake();
	void Collisions();



	void ValidateAdded();

	void ValidateRemoved();

	void addEntity(Entity* ent);
	void eraseEntity(Entity* ent);

	Entity* cloneEntity(Entity* ent);

	std::vector<SerializableEntity> SerializeEntities();

	std::size_t GetTotalEntities();

	std::vector<std::vector<std::string>> GetActiveCollisions();

	void AddColliders(std::vector<std::string>& Colliders);

	void RemoveActiveCollision(std::vector<std::string> it);

	void DisplayEntities();

	void ClearInspector();

	void DisplayComponents();

	void DestroyAllEntities();

private:
	std::vector<std::unique_ptr<Entity>> entities;
	std::vector<std::unique_ptr<Entity>> to_add;

	std::vector<std::vector<std::string>> activeCollisions;
};