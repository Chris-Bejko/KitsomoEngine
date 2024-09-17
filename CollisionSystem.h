#pragma once
#include "System.h"
#include "SFML/Graphics.hpp"
#include "BoxCollider2D.h";
#include "EntityManager.h"

class CollisionSystem : public System
{
public:
	CollisionSystem() = default;
	~CollisionSystem() = default;
	bool AABB(sf::FloatRect a, sf::FloatRect b);


	bool AABB(BoxCollider2D colA, BoxCollider2D colB);

	inline static CollisionSystem get()
	{
		if (s_instance == nullptr)
		{
			s_instance = new CollisionSystem();
		}

		return *s_instance;
	}

	void Update() override final;

private:
	static CollisionSystem* s_instance;

};