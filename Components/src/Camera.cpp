#include "Camera.h"
#include "Engine.h"
#include "ComponentRegistry.h"

DECLARE_COMPONENT_RULES(Camera, false)
REGISTER_COMPONENT(Camera)
REGISTER_SERIALIZABLE_COMPONENT(Camera)

bool Camera::Init()
{
	LOG_INFO("========== CAMERA INIT ==========");
    LOG_INFO("Camera this: ", this);
    LOG_INFO("Entity: ", entity);
    const auto windowSize = Engine::get().GetWindow().getSize();

    view.setSize(
        static_cast<float>(windowSize.x),
        static_cast<float>(windowSize.y));

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
