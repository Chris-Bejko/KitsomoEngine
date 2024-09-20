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


	bool AABB(BoxCollider colA, BoxCollider colB);

	inline static CollisionSystem get()
	{
		if (s_instance == nullptr)
		{
			s_instance = new CollisionSystem();
		}

		return *s_instance;
	}

	void Update() override final;

	bool ActiveCollision(std::string a, std::string b);

	void SetActive(std::string a, std::string b);
	void SetInactive(std::string a, std::string b);

private:
	static CollisionSystem* s_instance;


};


