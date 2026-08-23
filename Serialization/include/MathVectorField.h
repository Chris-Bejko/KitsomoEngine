#pragma once

#include "SerializedField.h"
#include "Vector2.h"
#include <sstream>
#include <type_traits>
#include <string>

template <typename T, typename Derived>
class MathVectorField : public SerializedField
{
private:
    VectorBase<T, Derived> *value;

public:
    MathVectorField(const char *name, VectorBase<T, Derived> *value) : SerializedField(name), value(value)
    {
    }
    void Draw(const FieldDrawContext &context) override;

    std::string Serialize() override;

    void Deserialize(const std::string &serialized) override;
};

template <typename T, typename Derived>
std::string MathVectorField<T, Derived>::Serialize()
{
    const T *data =
        reinterpret_cast<const T *>(value);

    std::string result;

    for (size_t i = 0; i < Derived::Size; ++i)
    {
        if (i > 0)
            result += "|";

        result += std::to_string(data[i]);
    }

    return result;
}

template <typename T, typename Derived>
void MathVectorField<T, Derived>::Deserialize(
    const std::string &serialized)
{
    T *data =
        reinterpret_cast<T *>(value);

    std::stringstream ss(serialized);
    std::string token;

    size_t index = 0;

    while (std::getline(ss, token, '|') &&
           index < Derived::Size)
    {
        try
        {
            if constexpr (std::is_integral_v<T>)
                data[index] = static_cast<T>(std::stoi(token));
            else
                data[index] = static_cast<T>(std::stof(token));

            ++index;
        }
        catch (...)
        {
            LOG_WARNING(
                "Failed to deserialize vector value: ",
                token.c_str());
        }
    }
}

template <typename T, typename Derived>
void MathVectorField<T, Derived>::Draw(
    const FieldDrawContext &context)
{
    T *data = reinterpret_cast<T *>(value);

    if constexpr (std::is_same_v<T, int>)
    {
        switch (Derived::Size)
        {
        case 2:
            ImGui::InputInt2(
                context.fieldId.c_str(),
                data);
            break;

        case 3:
            ImGui::InputInt3(
                context.fieldId.c_str(),
                data);
            break;

        case 4:
            ImGui::InputInt4(
                context.fieldId.c_str(),
                data);
            break;
        }
    }
    else if constexpr (std::is_same_v<T, float>)
    {
        switch (Derived::Size)
        {
        case 2:
            ImGui::InputFloat2(
                context.fieldId.c_str(),
                data);
            break;

        case 3:
            ImGui::InputFloat3(
                context.fieldId.c_str(),
                data);
            break;

        case 4:
            ImGui::InputFloat4(
                context.fieldId.c_str(),
                data);
            break;
        }
    }
}