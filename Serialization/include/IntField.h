#pragma once
#include "SerializedField.h"
#include "imgui.h"

class IntField : public SerializedField
{
    int *value;

public:
    IntField(const char *name, int *val) : SerializedField(name), value(val)
    {
    }
    void Draw(const FieldDrawContext &context) override;
    std::string Serialize() override
    {
        return std::to_string(*value);
    }
    void Deserialize(const std::string &input) override
    {
        try
        {
            *value = std::stoi(input);
        }
        catch (...)
        {
            *value = 0;
        }
    }
};