#include "EntityVectorField.h"
#include <type_traits>
#include "Engine.h"
#include "Entity.h"

void EntityVectorField::Resolve()
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

        bool found = false;
        for (auto &entity : Engine::get().GetManager()->GetEntities())
        {
            if (entity && entity->GetGUID() == token)
            {
                value->push_back(entity.get());
                found = true;
                break;
            }
        }
        if (!found)
        {
            for (auto &entity : Engine::get().GetManager()->GetUnvalidatedEntities())
            {
                if (entity && entity->GetGUID() == token)
                {
                    value->push_back(entity.get());
                    break;
                }
            }
        }
    }
}

void EntityVectorField::Draw(const FieldDrawContext& context)
{
    Resolve();

    int removeIndex = -1;

    for (size_t i = 0; i < value->size(); ++i)
    {
        std::string elementId = context.fieldId + "_" + std::to_string(i);

        ImGui::PushID(elementId.c_str());

        ImGui::Text("%d.", static_cast<int>(i + 1));
        ImGui::SameLine();

        std::string displayText = "Drop entity here...";

        Entity* entity = (*value)[i];

        if (entity)
            displayText = "-> " + entity->GetName();

        ImGui::Button(displayText.c_str(),ImVec2(-40, 0));

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY"))
            {
                Entity* dropped = *(Entity**)payload->Data;

                if (dropped)
                    (*value)[i] = dropped;
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

    if (ImGui::Button(("+ Add##" + context.fieldId).c_str(),ImVec2(-1, 0)))
    {
        value->push_back(nullptr);
    }
}


std::string EntityVectorField::Serialize()
{
    std::string result;

    for (size_t i = 0; i < value->size(); ++i)
    {
        if (i > 0)
            result += ";";

        Entity *ptr = (*value)[i];

        if (ptr == nullptr)
            continue;

        result += ptr->GetGUID();
    }

    return result;
}

void EntityVectorField::Deserialize(const std::string &serialized)
{
    serializedValue = serialized;

    value->clear();
}
