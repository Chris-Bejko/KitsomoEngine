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
    void AddInitialForce(Vector2F f);


    Vector2F GetForce();
    Vector2F GetInitialForce() { return initialForce; }
    Vector2F GetVelocity();
	
    
    
    float GetMass() { return mass; }
    void SetMass(float m) { mass = m; }

    float GetGravityScale() { return gravityScale; }
    void SetGravityScale(float g) { gravityScale = g; }

    float GetBounciness() { return bounciness; }
    void SetBounciness(float b) { bounciness = b; }

    float GetFriction() { return friction; }
    void SetFriction(float f) { friction = f; }

    bool IsStatic() { return isStatic; }        
    void SetStatic(bool s) { isStatic = s; }

    bool IsKinematic() { return isKinematic; }
    void SetKinematic(bool k) { isKinematic = k; }

    bool UsesGravity() { return useGravity; }
    void SetUseGravity(bool g) { useGravity = g; }

    
    void InitSerializedFields(ReadableSerializableVariableMap map) override final;
	std::vector<SerializableVariable>* GetSerializedFields() override final;
	void Serialize() override final;


private:
    float mass = 1.0f;
    float gravityScale = 1.0f;
    float bounciness = 0.0f;
    float friction = 0.5f; 
    bool isStatic = false;
    bool isKinematic = false;
    bool useGravity = true;


    Vector2F drag = Vector2F();
    Vector2F force = Vector2F();
    bool firstUpdate = true;
    Vector2F initialForce;
    Vector2F velocity = Vector2F();
    std::vector<SerializableVariable> serializables;
};