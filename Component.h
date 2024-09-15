#pragma once

#include <SFML/Graphics.hpp>

class Entity;

class Component
{
public:
	Component() = default;
	virtual ~Component() = default;
	Entity* entity;

	virtual bool Init() { return true; }
	virtual void draw() { }
	virtual void update() { }
};