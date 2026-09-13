#pragma once
#include "SerializedField.h"
#include "imgui.h"

class UIntField : public SerializedField
{
    unsigned int *value;

public:
    UIntField(const char *name, unsigned int *val) : SerializedField(name), value(val)
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
            *value = static_cast<unsigned int>(std::stoul(input));
        }
        catch (...)
        {
            *value = 0;
        }
    }
};