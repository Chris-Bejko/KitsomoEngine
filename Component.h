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
	virtual void OnCollisionExit(BoxCollider2D& other) { }
	virtual void OnTriggerEnter(BoxCollider2D& other) { }
	virtual void OnTriggerExit(BoxCollider2D& other) { }
	virtual void OnTriggerStay(BoxCollider2D& other) { }
};