#pragma once
#include "../Entity.h"
#include "../Vector2.h"
#include "../Component.h"

constexpr float GRAVITY = 10.0f;

class Rigidbody : public Component
{
public:
    Rigidbody();
    virtual ~Rigidbody() = default;
    Rigidbody(float gravityScale);

    bool Init() override final;

    void update(float dt) override final;
    void SetForce(const Vector2F force);

    void SetForce(float x, float y);

    void AddForce(const Vector2F force);

    Vector2F GetForce();
    Vector2F GetVelocity();
    Vector2F GetInitialForce() { return initialForce; }
    void AddInitialForce(Vector2F f);

private:
    float mass = 1.0f;
    float gravityScale = 1.0f;
    Vector2F drag = Vector2F();
    Vector2F force = Vector2F();
    bool firstUpdate = true;
    Vector2F initialForce;

    Vector2F velocity = Vector2F();
};