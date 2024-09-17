#pragma once

#include <SFML/Graphics.hpp>

class Entity;
class BoxCollider2D;


class Component
{
public:
	Component() = default;
	virtual ~Component() = default;
	Entity* entity;

	virtual bool Init() { return true; }
	virtual void draw() { }
	virtual void update() { }

	virtual void OnCollisionEnter(BoxCollider2D& other) { }
};