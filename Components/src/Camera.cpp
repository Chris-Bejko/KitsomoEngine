#include "Camera.h"
#include "Engine.h"
#include "ComponentRegistry.h"

DECLARE_COMPONENT_RULES(Camera, false)
REGISTER_SERIALIZABLE_COMPONENT(Camera)

bool Camera::Init()
{
	sf::View view;
	sf::Vector2f size(Engine::get().GetWindow().getSize().x, Engine::get().GetWindow().getSize().y);
	view.setSize(size);
	this->view = view;
	transform = &entity->GetComponent<Transform>();
	return true;
}

void Camera::update(float dt)
{
	Engine::get().SetView(view);
}

void Camera::Follow(sf::Vector2f position)
{
	view.setCenter(position.x, position.y);
}
