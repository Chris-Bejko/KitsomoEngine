#include "EntityRefField.h"
#include "Engine.h"

std::string EntityRefField::Serialize() 
{
    if (value == nullptr || *value == nullptr)
        return "";

    return (*value)->GetGUID();
}


void EntityRefField::Deserialize(const std::string& serialized)
{
    guidStorage = serialized;
    if (value)
        *value = nullptr;
}


void EntityRefField::Resolve()
{
    if (!value)
        return;

    if (guidStorage.empty() || guidStorage == "null")
    {
        *value = nullptr;
        return;
    }

    *value = nullptr;

    if (!Engine::get().GetManager())
        return;

    for (auto& entity : Engine::get().GetManager()->GetEntities())
    {
        if (entity && entity->GetGUID() == guidStorage)
        {
            *value = entity.get();
            return;
        }
    }
    for (auto& entity : Engine::get().GetManager()->GetUnvalidatedEntities())
    {
        if (entity && entity->GetGUID() == guidStorage)
        {
            *value = entity.get();
            return;
        }
    }
}

void EntityRefField::Draw(const FieldDrawContext& context)
{
    Resolve();

    Entity* resolved = value ? *value : nullptr;
    std::string displayText = resolved ? "-> " + resolved->GetName() : guidStorage.empty() ? "Drop entity here..." : "NOT FOUND";
    ImVec4 boxColor = resolved ? ImVec4(0.1f, 0.25f, 0.1f, 1.0f) : guidStorage.empty() ? ImVec4(0.15f, 0.18f, 0.25f, 1.0f) : ImVec4(0.35f, 0.1f, 0.1f, 1.0f);

    ImGui::PushStyleColor(ImGuiCol_Button, boxColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(boxColor.x + 0.05f, boxColor.y + 0.05f, boxColor.z + 0.1f, 1.0f));
    ImGui::Text("%s", name);
    ImGui::Button((displayText + "##" + context.fieldId).c_str(), ImVec2(-1, 0));
    ImGui::PopStyleColor(2);

    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY"))
        {
            Entity* dropped = *(Entity**)payload->Data;
            if (dropped)
            {
                *value = dropped;
                guidStorage = dropped->GetGUID();
                Engine::get().GetManager()->SetSelectedEntity(context.entity);
            }
        }
        else if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("PREFAB"))
        {
            const char* pathStr = static_cast<const char*>(payload->Data);
            if (pathStr)
            {
                Entity* spawned = Engine::get().SpawnPrefab(pathStr, Vector2F(0, 0));
                if (spawned)
                {
                    *value = spawned;
                    guidStorage = spawned->GetGUID();
                    Engine::get().GetManager()->SetSelectedEntity(context.entity);
                }
            }
        }
        ImGui::EndDragDropTarget();
    }
}