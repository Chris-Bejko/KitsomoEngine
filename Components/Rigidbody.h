#pragma once
#include "../Entity.h"
#include "../Vector2.h"
#include "../Component.h"

constexpr float GRAVITY = 10.0f;

class Rigidbody : public Component
{
public:
    Rigidbody()
    {
        gravityScale = 1.0f;
    }
    virtual ~Rigidbody() = default;
    Rigidbody(float gravityScale)
    {
        this->gravityScale = gravityScale;
    }

    bool Init() override final
    {
        transform = &entity->GetComponent<Transform>();
        return true;
    }

    void update(float dt) override final
    {
        velocity.x = force.x - drag.x * dt;
        velocity.y = force.y + drag.y + gravityScale * GRAVITY * mass * dt;
        transform->Translate(velocity);
    }

    void SetForce(const Vector2F force)
    {
        this->force = force;
    }

    void SetForce(float x, float y)
    {
        this->force.x = x;
        this->force.y = y;
    }
    void AddForce(const Vector2F force)
    {
        this->force += force;
    }

    Vector2F GetVelocity()
    {
        return velocity;
    }
private:
    float mass = 1.0f;
    float gravityScale = 1.0f;
    Vector2F drag = Vector2F();
    Vector2F force = Vector2F();

    Vector2F velocity = Vector2F();
    Transform* transform = nullptr;
};