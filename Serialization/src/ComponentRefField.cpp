#include "ComponentRefField.h"
#include "Engine.h"

std::string ComponentRefField::Serialize()
{
    if (value == nullptr || *value == nullptr)
        return "";

    Component *component = *value;

    if (component->entity == nullptr)
        return "";

    return component->entity->GetGUID() + "|" + component->GetGUID();
}

void ComponentRefField::Deserialize(const std::string &serialized)
{
    packedGUID = serialized;

    if (value)
        *value = nullptr;
}

void ComponentRefField::Resolve()
{
    if (!value)
        return;

    *value = nullptr;

    if (packedGUID.empty())
        return;

    auto pipe = packedGUID.find('|');

    if (pipe == std::string::npos)
        return;

    std::string entityGUID = packedGUID.substr(0, pipe);

    std::string componentGUID = packedGUID.substr(pipe + 1);

    for (auto &entity : Engine::get().GetManager()->GetEntities())
    {
        if (entity->GetGUID() != entityGUID)
            continue;

        for (auto &component : entity->GetComponents())
        {
            if (component->GetGUID() == componentGUID)
            {
                *value = component.get();
                return;
            }
        }

        return;
    }
}

void ComponentRefField::Draw(const FieldDrawContext &context)
{
    bool isEmpty = packedGUID.empty() || packedGUID == "null";

    Component *resolved = nullptr;
    ImGui::Text("%s", name);

    if (!isEmpty)
    {
        auto pipe = packedGUID.find('|');

        if (pipe != std::string::npos)
        {
            std::string entityGUID = packedGUID.substr(0, pipe);

            std::string componentGUID = packedGUID.substr(pipe + 1);

            for (auto &entity : Engine::get().GetManager()->GetEntities())
            {
                if (entity->GetGUID() != entityGUID)
                    continue;

                for (auto &component : entity->GetComponents())
                {
                    if (component->GetGUID() == componentGUID)
                    {
                        resolved = component.get();
                        break;
                    }
                }

                break;
            }
        }
    }

    std::string displayText = "Drop component here...";

    if (resolved)
    {
        std::string compName = typeid(*resolved).name();

        compName = std::regex_replace(compName, std::regex("class "), "");

        displayText = compName + " (" + resolved->entity->GetName() + ")";
    }
    else if (!isEmpty)
    {
        displayText = "NOT FOUND";
    }

    ImVec4 boxColor = resolved ? ImVec4(0.1f, 0.25f, 0.1f, 1.0f) : !isEmpty ? ImVec4(0.35f, 0.1f, 0.1f, 1.0f)
                                                                            : ImVec4(0.15f, 0.18f, 0.25f, 1.0f);

    ImGui::PushStyleColor(ImGuiCol_Button, boxColor);

    ImGui::Button(
        (displayText + "##" + context.fieldId).c_str(),
        ImVec2(-40, 0));

    ImGui::PopStyleColor();

    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("ENTITY"))
        {
            Entity *dropped = *(Entity **)payload->Data;

            if (dropped)
            {
                for (auto &component : dropped->GetComponents())
                {
                    std::string componentTypeName = typeid(*component).name();

                    componentTypeName = std::regex_replace(componentTypeName, std::regex("class "), "");

                    if (componentTypeName == "Transform" || componentTypeName == "EditorSprite")
                        continue;

                    if (typeHint.empty() || componentTypeName == typeHint)
                    {
                        packedGUID = dropped->GetGUID() + "|" + component->GetGUID();
                        Engine::get().GetManager()->SetSelectedEntity(context.entity);
                        break;
                    }
                }
            }
        }

        ImGui::EndDragDropTarget();
    }
}