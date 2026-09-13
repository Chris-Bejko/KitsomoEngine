#pragma once
#include "SerializedField.h"
#include <regex>
#include <sstream>
#include <typeinfo>
#include <vector>
#include "imgui.h"
template <typename T>
class ComponentVectorField : public SerializedField
{
private:
    std::vector<T *> *value;
    std::string typeHint;
    std::string serializedValue;

public:
    ComponentVectorField(const char *name, std::vector<T *> *value, const std::string &typeHint) : SerializedField(name), value(value), typeHint(typeHint)
    {
    }

    std::string Serialize() const override;

    void Deserialize(const std::string &serialized) override;
    void Resolve() override;

    void Draw(const FieldDrawContext &context) override;
};


template <typename T>
void ComponentVectorField<T>::Draw(const FieldDrawContext &context)
{
    Resolve();

    int removeIndex = -1;

    for (size_t i = 0; i < value->size(); ++i)
    {
        std::string elementId = context.fieldId + "_" + std::to_string(i);

        ImGui::PushID(elementId.c_str());

        ImGui::Text("%d.", static_cast<int>(i + 1));
        ImGui::SameLine();

        std::string displayText = "Drop component here...";

        T *component = (*value)[i];

        if (component && component->entity)
        {
            std::string componentName = typeid(*component).name();

            componentName = std::regex_replace(componentName, std::regex("class "), "");

            displayText = componentName + " (" + component->entity->GetName() + ")";
        }

        ImGui::Button(displayText.c_str(), ImVec2(-40, 0));

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("ENTITY"))
            {
                Entity *dropped = *(Entity **)payload->Data;

                if (dropped)
                {
                    for (auto &comp : dropped->GetComponents())
                    {
                        std::string componentType = typeid(*comp).name();

                        componentType = std::regex_replace(componentType, std::regex("class "), "");

                        if (componentType == "Transform" || componentType == "EditorSprite")
                            continue;

                        if (typeHint.empty() || componentType == typeHint)
                        {
                            T *typed = dynamic_cast<T *>(comp.get());

                            if (typed)
                                (*value)[i] = typed;

                            break;
                        }
                    }
                }
            }

            ImGui::EndDragDropTarget();
        }

        ImGui::SameLine();

        if (ImGui::Button("X"))
            removeIndex = static_cast<int>(i);

        ImGui::PopID();
    }

    if (removeIndex >= 0)
        value->erase(value->begin() + removeIndex);

    if (ImGui::Button(("+ Add##" + context.fieldId).c_str(), ImVec2(-1, 0)))
    {
        value->push_back(nullptr);
    }
}

template <typename T>
void ComponentVectorField<T>::Deserialize(const std::string &serialized)
{
    serializedValue = serialized;
    value->clear();
}

template <typename T>
std::string ComponentVectorField<T>::Serialize() const
{
    std::string result;

    for (size_t i = 0; i < value->size(); ++i)
    {
        if (i > 0)
            result += ";";

        if ((*value)[i] == nullptr)
            continue;

        result += (*value)[i]->entity->GetGUID() + "|" + (*value)[i]->GetGUID();
    }

    return result;
}

template <typename T>
void ComponentVectorField<T>::Resolve()
{
    if (!value)
        return;

    value->clear();

    if (serializedValue.empty())
        return;

    std::stringstream ss(serializedValue);
    std::string token;

    if (!Engine::get().GetManager())
        return;

    while (std::getline(ss, token, ';'))
    {
        if (token.empty() || token == "null")
            continue;

        auto pipe = token.find('|');

        if (pipe == std::string::npos)
            continue;

        std::string entityGUID = token.substr(0, pipe);
        std::string componentGUID = token.substr(pipe + 1);

        auto checkEntity = [&](Entity *entity) -> bool {
            if (!entity || entity->GetGUID() != entityGUID)
                return false;

            for (auto &component : entity->GetComponents())
            {
                if (component && component->GetGUID() == componentGUID)
                {
                    T *typed = dynamic_cast<T *>(component.get());
                    if (typed)
                    {
                        value->push_back(typed);
                        return true;
                    }
                }
            }
            for (auto &component : entity->GetUnvalidatedComponents())
            {
                if (component && component->GetGUID() == componentGUID)
                {
                    T *typed = dynamic_cast<T *>(component.get());
                    if (typed)
                    {
                        value->push_back(typed);
                        return true;
                    }
                }
            }
            return false;
        };

        bool found = false;
        for (auto &entity : Engine::get().GetManager()->GetEntities())
        {
            if (checkEntity(entity.get()))
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            for (auto &entity : Engine::get().GetManager()->GetUnvalidatedEntities())
            {
                if (checkEntity(entity.get()))
                {
                    break;
                }
            }
        }
    }
}