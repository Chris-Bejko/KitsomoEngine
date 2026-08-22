#pragma once
#include "Transform.h"
#include "Component.h"
#include <SFML/Graphics.hpp>

class Camera : public SerializableScript
{
public:
    bool Init() override final;

    void update(float dt) override final;
    void Follow(sf::Vector2f target);

private:
    Transform* transform = nullptr;
    Transform* followTarget = nullptr;

    sf::View view;

};
