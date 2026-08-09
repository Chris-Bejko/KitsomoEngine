#pragma once
#include "Transform.h"
#include "Component.h"
#include <SFML/Graphics.hpp>

class Camera : public SerializableScript
{
public:
    Transform* transform;

    bool Init() override final;

    inline static Camera& main()
    {
        if (s_instance == nullptr)
        {
            s_instance = new Camera();
        }

        return *s_instance;
    }

    void update(float dt) override final;
    void Follow(sf::Vector2f target);

private:
    Transform* followTarget;
    sf::View view;
    static Camera* s_instance;

};