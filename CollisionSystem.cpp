#include "CollisionSystem.h"
#include <algorithm>
#include "Engine.h"

CollisionSystem* CollisionSystem::s_instance = nullptr;

bool CollisionSystem::AABB(sf::FloatRect a, sf::FloatRect b)
{
	return (a.left + a.width > b.left &&
		b.left + a.width > a.left &&
		b.top + b.height > a.top &&
		a.top + a.height > b.top);
}

bool CollisionSystem::AABB(BoxCollider colA, BoxCollider colB)
{
	return AABB(colA.GetRect(), colB.GetRect());
}

void CollisionSystem::Update()
{

}

bool CollisionSystem::ActiveCollision(std::string a, std::string b)
{
	if (a == b)
		return false;

	auto activeCollisions = Engine::get().GetManager()->GetActiveCollisions();
	for (auto& active : activeCollisions)
	{
		auto& val1 = active[0];
		auto& val2 = active[1];
		if ((val1 == a && val1 == b) || (val2 == a && val1 == b))
		{
			return true;
		}
	}


	return false;
}

void CollisionSystem::SetActive(std::string a, std::string b)
{
	if (a == b)
		return;
	auto activeCollisions = Engine::get().GetManager()->GetActiveCollisions();
	std::vector<std::string> colliders = { a, b };
	std::vector<std::string> collidersOpposite = { b, a };
	if (std::find(activeCollisions.begin(), activeCollisions.end(), colliders) != activeCollisions.end() || std::find(activeCollisions.begin(), activeCollisions.end(), collidersOpposite) != activeCollisions.end())
		return;
	Engine::get().GetManager()->AddColliders(colliders);

}

void CollisionSystem::SetInactive(std::string a, std::string b)
{
	if (a == b)
		return;
	auto activeCollisions = Engine::get().GetManager()->GetActiveCollisions();
	std::vector<std::string> vectors = { a,b };
	std::vector<std::string> vectors2 = { b,a };
	Engine::get().GetManager()->RemoveActiveCollision(vectors);
	Engine::get().GetManager()->RemoveActiveCollision(vectors2);
}