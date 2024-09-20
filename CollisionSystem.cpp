#include "CollisionSystem.h"
#include <algorithm>

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
		//std::cout << "Checking collision: " << active->a << " == " << a << "," << val2<< " == " << b << " is active." << std::endl;
		if ((val1 == a && val1 == b) || (val2 == a && val1 == b))
		{
			//std::cout << active->a << " == " << b << "," << val2<< " == " << a << std::endl;
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
	//std::cout << "Setting collision: " << &tuple.a << " , " << &tuple.b << " active." << std::endl;
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

	/*int k = -1;
	int j = -1;
	for (int i = 0; i < activeCollisions.size() - 1; i++)
	{
		auto& e = activeCollisions[i];
		auto& val1 = e[0];
		auto& val2 = e[1];
		if (val1 == a && val2 == b)
		{
			k = i;
		}
		if (val1 == b && val2 == a)
		{
			j = i;
		}
	}

	if (j != -1)
		activeCollisions.erase(activeCollisions.begin() + j);
	if (k != -1)
		activeCollisions.erase(activeCollisions.begin() + k);*/



}