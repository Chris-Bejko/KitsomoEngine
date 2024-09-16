#include "CollisionSystem.h"

CollisionSystem* CollisionSystem::s_instance = nullptr;

bool CollisionSystem::AABB(sf::FloatRect a, sf::FloatRect b)
{
	return (a.left + a.width > b.left &&
		b.left + a.width > a.left &&
		b.top + b.height > a.top &&
		a.top + a.height > b.top);
}

bool CollisionSystem::AABB(BoxCollider2D& colA, BoxCollider2D& colB)
{
	return AABB(colA.GetRect(), colB.GetRect());
}

void CollisionSystem::Update()
{

}
