#pragma once
#include "System.h"
#include <string>
#include "SFML/Graphics.hpp"
#include <set>

class Collider;
class CollisionSystem : public System
{
public:
	CollisionSystem() = default;
	~CollisionSystem() = default;
	// CollisionSystem.h
	bool CheckCollision(Collider *a, Collider *b);
	bool AABB(sf::FloatRect a, sf::FloatRect b);
	// bool CircleVsCircle(sf::Vector2f posA, float rA, sf::Vector2f posB, float rB);
	bool AABBvsCircle(sf::FloatRect box, sf::Vector2f circlePos, float radius);

	// bool AABB(BoxCollider colA, BoxCollider colB);

	inline static CollisionSystem get()
	{
		if (s_instance == nullptr)
		{
			s_instance = new CollisionSystem();
		}

		return *s_instance;
	}

	void Update() override final;

	bool ActiveCollision(Collider*a	, Collider*b);

	void SetActive(Collider* a, Collider* b);
	void SetInactive(Collider* a, Collider* b);

	void ResolveCollision(Collider* a, Collider* b);
	sf::Vector2f GetCollisionNormal(Collider* a, Collider* b);
	float GetPenetrationDepth(Collider* a, Collider* b);	

private:
	static CollisionSystem *s_instance;
	std::set<std::pair<Collider*, Collider*>> activeCollisions;
};
