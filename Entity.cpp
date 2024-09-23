#include "Entity.h"

Entity::Entity(std::string name)
{
	this->transform = &this->AddComponent<Transform>(0, 0);
	isActive = true;
	SaveAvailableComponents();
	this->entityName = name;
}
