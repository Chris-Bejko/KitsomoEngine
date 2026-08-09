#pragma once

#include <vector>
#include <memory>
#include "Entity.h"
#include "imgui.h"
#include "imgui-SFML.h"
#include <set>

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

	void addEntity(Entity *ent);
	void eraseEntity(Entity *ent);

	Entity *cloneEntity(Entity *ent);

	std::vector<SerializableEntity> SerializeEntities();

	std::size_t GetTotalEntities();

	std::vector<std::vector<std::string>> GetActiveCollisions();

	void AddColliders(std::vector<std::string> &Colliders);

	void RemoveActiveCollision(std::vector<std::string> it);

	void DisplayEntities();

	void ClearInspector();

	void DisplayComponents();

	void DestroyAllEntities();
	void RemoveCollisionPairsForEntity(Entity *e);

	bool IsInColliderEditMode();
	bool cmp(std::pair<Entity *, int> &a,
			 std::pair<Entity *, int> &b)
	{
		return a.second < b.second;
	}
	Entity *GetSelectedEntity() { return selectedEntity; }
	void SetSelectedEntity(Entity *entity);

	std::string GetUniqueName(const std::string &baseName);
	std::vector<std::unique_ptr<Entity>> &GetEntities() { return entities; }
	void DisplayEntityNode(Entity *e);
	Entity *GetDragHoveredEntity() { return dragHoveredEntity; }
	void DisplayComponentsOf(Entity *e);
	void RemoveEntityByGUID(const std::string &guid);

private:
	// Track active collision pairs to ensure OnTriggerEnter/Stay/Exit called correctly
	std::set<std::pair<Collider *, Collider *>> activeCollisionPairs;

private:
	std::vector<std::unique_ptr<Entity>> entities;
	std::vector<std::unique_ptr<Entity>> to_add;

	std::vector<std::vector<std::string>> activeCollisions;
	Entity *selectedEntity = nullptr;
	Entity *lastClickedEntity = nullptr;
	float lastClickTime = 0.f;
	Entity *dragHoveredEntity = nullptr;
};