#pragma once
#include "SerializedField.h"
#include <vector>
#include <string>

class Entity;

class EntityVectorField : public SerializedField
{
private:
    std::vector<Entity *> *value;
    std::string typeHint;
    std::string serializedValue;

public:
    EntityVectorField(const char *name, std::vector<Entity *> *value, const std::string &typeHint = "") : SerializedField(name), value(value), typeHint(typeHint)
    {
    }

    void Draw(const FieldDrawContext &context) override;
    std::string Serialize() override;

    void Deserialize(const std::string &serialized) override;
    void Resolve() override;
};