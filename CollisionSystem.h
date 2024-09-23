#pragma once
#include "System.h"
#include <string>
#include "SFML/Graphics.hpp"


class CollisionSystem : public System
{
public:
	CollisionSystem() = default;
	~CollisionSystem() = default;
	bool AABB(sf::FloatRect a, sf::FloatRect b);


	//bool AABB(BoxCollider colA, BoxCollider colB);

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


