#pragma once

#include <vector>
#include <memory>
#include "Entity.h"
#include "imgui.h"
#include "imgui-SFML.h"

class EntityManager
{
public:
	EntityManager() = default;
	~EntityManager() = default;

	void draw();
	void update(float dt);
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

	void DisplayEntities()
	{
		for (auto& e : entities)
		{
			if(ImGui::Checkbox(e->GetName().c_str(), &e->displayComponents))
			{
				for(auto& a : entities)
				{
					if(a->GetName() == e->GetName())
						continue;

					a->displayComponents = false;
				}
			}

		}
	}

	void DisplayComponents()
	{
		for (auto& e : entities)
		{
			e->DisplayComponents();
		}
	}


private:
	std::vector<std::unique_ptr<Entity>> entities;
	std::vector<std::unique_ptr<Entity>> to_add;

	std::vector<std::vector<std::string>> activeCollisions;
};