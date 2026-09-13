#pragma once

#include "SerializedField.h"

class EntityRefField : public SerializedField
{
private:
    Entity **value;
    std::string guidStorage;

public:
    EntityRefField(const char *name, Entity *&value) : SerializedField(name), value(&value)
    {
    }

    void Draw(const FieldDrawContext &context) override;

    std::string Serialize() override;

    void Deserialize(const std::string &serialized) override;

    void Resolve() override;
};