#include "CollisionSystem.h"
#include <algorithm>
#include "../Engine.h"

CollisionSystem *CollisionSystem::s_instance = nullptr;

bool CollisionSystem::AABB(sf::FloatRect a, sf::FloatRect b)
{
	return (a.left + a.width > b.left &&
			b.left + a.width > a.left &&
			b.top + b.height > a.top &&
			a.top + a.height > b.top);
}

void CollisionSystem::Update()
{
}
bool CollisionSystem::CheckCollision(Collider *a, Collider *b)
{
	if (!a || !b)
		return false;

	auto typeA = a->GetType();
	auto typeB = b->GetType();

	if (typeA == ColliderType::Box && typeB == ColliderType::Box)
		return AABB(a->GetBounds(), b->GetBounds());

	if (typeA == ColliderType::Circle && typeB == ColliderType::Circle)
		return a->Intersects(*b);

	if ((typeA == ColliderType::Box && typeB == ColliderType::Circle) ||
		(typeA == ColliderType::Circle && typeB == ColliderType::Box))
	{
		Collider *box = typeA == ColliderType::Box ? a : b;
		Collider *circle = typeA == ColliderType::Circle ? a : b;

		sf::FloatRect bounds = circle->GetBounds();
		sf::Vector2f center(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);
		float radius = bounds.width / 2.f;

		return AABBvsCircle(box->GetBounds(), center, radius);
	}

	return false;
}

bool CollisionSystem::AABBvsCircle(sf::FloatRect box, sf::Vector2f circlePos, float radius)
{
	// Find closest point on box to circle center
	float closestX = std::clamp(circlePos.x, box.left, box.left + box.width);
	float closestY = std::clamp(circlePos.y, box.top, box.top + box.height);

	float dx = circlePos.x - closestX;
	float dy = circlePos.y - closestY;

	return (dx * dx + dy * dy) < (radius * radius);
}

bool CollisionSystem::ActiveCollision(Collider *a, Collider *b)
{
	return activeCollisions.count({a, b}) || activeCollisions.count({b, a});
}

void CollisionSystem::SetActive(Collider *a, Collider *b)
{
	activeCollisions.insert({a, b});
}

void CollisionSystem::SetInactive(Collider *a, Collider *b)
{
	activeCollisions.erase({a, b});
	activeCollisions.erase({b, a});
}