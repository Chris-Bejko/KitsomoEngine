#pragma once
#include "../Component.h"
#include "Vector2.h"

class Transform : public Component
{
public:
    Transform() = default;
    Transform(float x, float y)
    {
        position.x = x;
        position.y = y;
    }

    Transform(float x, float y, float scX, float scY)
    {
        position.x = x;
        position.y = y;
        scale.x = scX;
        scale.y = scY;
        rotation = 0;
    }

    Transform(float x, float y, float scX, float scY, float roation)
    {
        position.x = x;
        position.y = y;
        scale.x = scX;
        scale.y = scY;
        this->rotation = rotation;
    }
    virtual ~Transform() = default;

    Vector2F position = Vector2F();
    Vector2F scale = Vector2F(1.0f, 1.0f);
    float rotation = 0.0f;

    void Translate(const Vector2F velocity)
    {
        position += velocity;
    }
};