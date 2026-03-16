#pragma once
#include "../Component.h"
#include "../Serialization.h"
#include "SFML/Graphics.hpp"
#include <string>

enum class ColliderType
{
    Box,
    Circle,
    Polygon
};

class Collider : public Component
{
public:
    Collider() = default;
    virtual ~Collider() = default;

    // Must implement
    virtual bool Intersects(Collider& other) = 0;
    virtual sf::FloatRect GetBounds() = 0;
    virtual void DrawDebug() = 0;
    virtual ColliderType GetType() = 0;

    // Shared behaviour
    std::string GetCollisionTag() { return collisionTag; }
    void SetCollisionTag(const std::string& tag) { collisionTag = tag; }
    bool IsTrigger() { return isTrigger; }
    bool editMode = false;

    virtual void Serialize() {}
    virtual std::vector<SerializableVariable>* GetSerializedFields() override { return &serializables; }
    virtual void InitSerializedFields(ReadableSerializableVariableMap map) {}

    virtual void DrawEditorButton() override;

    // Collision callbacks forwarded from Entity
    void OnCollisionEnter(Collider& other) override {}
    void OnCollisionExit(Collider& other) override {}
    void OnTriggerEnter(Collider& other) override {}
    void OnTriggerExit(Collider& other) override {}
    void OnTriggerStay(Collider& other) override {}
	bool IsInEditMode() { return editMode; }

protected:
    std::string collisionTag = "";
    bool isTrigger = false;
    bool isEditMode = false;
    std::vector<SerializableVariable> serializables;
};