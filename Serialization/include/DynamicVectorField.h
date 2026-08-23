#pragma once

#include "SerializedField.h"

#include <vector>
#include <string>
#include <sstream>
#include <type_traits>

template <typename T>
class DynamicVectorField : public SerializedField
{
private:
    std::vector<T> *value;

public:
    DynamicVectorField(
        const char *name,
        std::vector<T> *value)
        : SerializedField(name),
          value(value)
    {
    }
    void Draw(const FieldDrawContext &context) override;

    std::string Serialize() override;

    void Deserialize(const std::string &serialized) override;
};
///TODO: Factory pattern for Vectors: input type -> output TypeField, so that VectorField<T> can just call Draw of each TypeField
template <typename T>
void DynamicVectorField<T>::Draw(const FieldDrawContext &context)
{
    int removeIndex = -1;
    LOG_DEBUG("DynamicVectorField T = ", typeid(T).name());
    for (size_t i = 0; i < value->size(); ++i)
    {
        std::string id =
            context.fieldId + "_" + std::to_string(i);

        ImGui::PushID(id.c_str());

        ImGui::Text("%d.", static_cast<int>(i + 1));
        ImGui::SameLine();

        if constexpr (std::is_same_v<T, int>)
        {
            int temp = (*value)[i];

            ImGui::SetNextItemWidth(-40);

            if (ImGui::InputInt("##value", &temp))
                (*value)[i] = temp;
        }
        else if constexpr (std::is_same_v<T, float>)
        {
            float temp = (*value)[i];

            ImGui::SetNextItemWidth(-40);

            if (ImGui::InputFloat("##value", &temp))
                (*value)[i] = temp;
        }
        else if constexpr (std::is_same_v<T, bool>)
        {
            bool temp = (*value)[i];

            if (ImGui::Checkbox("##value", &temp))
                (*value)[i] = temp;
        }
        else if constexpr (std::is_same_v<T, std::string>)
        {
            char buffer[200]{};
            strncpy_s(
                buffer,
                (*value)[i].c_str(),
                sizeof(buffer) - 1);

            ImGui::SetNextItemWidth(-40);

            if (ImGui::InputText(
                    "##value",
                    buffer,
                    sizeof(buffer)))
            {
                (*value)[i] = buffer;
            }
        }else if constexpr (std::is_same_v<T, Vector2F>)
        {
            Vector2F temp = (*value)[i];

            ImGui::SetNextItemWidth(-40);

            if (ImGui::InputFloat2("##value", &temp.x))
                (*value)[i] = temp;
        }

        ImGui::SameLine();

        if (ImGui::Button("X"))
            removeIndex = static_cast<int>(i);

        ImGui::PopID();
    }

    if (removeIndex >= 0)
        value->erase(value->begin() + removeIndex);

    if (ImGui::Button(
            ("+ Add##" + context.fieldId).c_str(),
            ImVec2(-1, 0)))
    {
        value->push_back(T{});
    }
}

template <typename T>
std::string DynamicVectorField<T>::Serialize()
{
    std::string result;

    for (size_t i = 0; i < value->size(); ++i)
    {
        if (i > 0)
            result += ";";

        if constexpr (std::is_same_v<T, bool>)
        {
            result += (*value)[i] ? "1" : "0";
        }
        else if constexpr (std::is_arithmetic_v<T>)
        {
            result += std::to_string((*value)[i]);
        }else if constexpr (std::is_same_v<T, Vector2F>)
        {
            result += std::to_string((*value)[i].x) + "|" + std::to_string((*value)[i].y);
        }
        else
        {
            result += (*value)[i];
        }
    }

    return result;
}

template <typename T>
void DynamicVectorField<T>::Deserialize(const std::string &serialized)
{
    value->clear();

    if (serialized.empty())
        return;

    std::stringstream ss(serialized);
    std::string token;

    while (std::getline(ss, token, ';'))
    {
        if (token.empty())
            continue;

        try
        {
            if constexpr (std::is_same_v<T, int>)
            {
                value->push_back(std::stoi(token));
            }
            else if constexpr (std::is_same_v<T, float>)
            {
                value->push_back(std::stof(token));
            }
            else if constexpr (std::is_same_v<T, bool>)
            {
                value->push_back(token == "1");
            }
            else if constexpr (std::is_same_v<T, std::string>)
            {
                value->push_back(token);
            }else if constexpr (std::is_same_v<T, Vector2F>)
            {
                std::stringstream vecStream(token);
                std::string xStr, yStr;
                if (std::getline(vecStream, xStr, '|') && std::getline(vecStream, yStr))
                {
                    float x = std::stof(xStr);
                    float y = std::stof(yStr);
                    value->push_back(Vector2F{x, y});
                }
            }
        }
        catch (...)
        {
            LOG_WARNING(
                "Failed to deserialize vector element: ",
                token.c_str());
        }
    }
}