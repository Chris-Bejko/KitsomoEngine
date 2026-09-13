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
}

void ComponentRefField::Resolve()
{
    if (!value)
        return;

    *value = nullptr;

    if (packedGUID.empty() || packedGUID == "null")
        return;

    auto pipe = packedGUID.find('|');

    if (pipe == std::string::npos)
        return;

    std::string entityGUID = packedGUID.substr(0, pipe);
    std::string componentGUID = packedGUID.substr(pipe + 1);

    if (!Engine::get().GetManager())
        return;

    auto checkEntity = [&](Entity *entity) -> bool {
        if (!entity || entity->GetGUID() != entityGUID)
            return false;

        for (auto &component : entity->GetComponents())
        {
            if (component && component->GetGUID() == componentGUID)
            {
                *value = component.get();
                return true;
            }
        }
        for (auto &component : entity->GetUnvalidatedComponents())
        {
            if (component && component->GetGUID() == componentGUID)
            {
                *value = component.get();
                return true;
            }
        }
        return false;
    };

    for (auto &entity : Engine::get().GetManager()->GetEntities())
    {
        if (checkEntity(entity.get()))
            return;
    }
    for (auto &entity : Engine::get().GetManager()->GetUnvalidatedEntities())
    {
        if (checkEntity(entity.get()))
            return;
    }
}

void ComponentRefField::Draw(const FieldDrawContext &context)
{
    Resolve();

    bool isEmpty = packedGUID.empty() || packedGUID == "null";
    Component *resolved = value ? *value : nullptr;

    ImGui::Text("%s", name);

    std::string displayText = "Drop component here...";

    if (resolved && resolved->entity)
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
                auto checkComps = [&](const std::vector<std::unique_ptr<Component>> &compList) -> bool {
                    for (auto &component : compList)
                    {
                        if (!component)
                            continue;
                        std::string componentTypeName = typeid(*component).name();
                        componentTypeName = std::regex_replace(componentTypeName, std::regex("class "), "");

                        if (componentTypeName == "Transform" || componentTypeName == "EditorSprite")
                            continue;

                        if (typeHint.empty() || componentTypeName == typeHint)
                        {
                            packedGUID = dropped->GetGUID() + "|" + component->GetGUID();
                            *value = component.get();
                            Engine::get().GetManager()->SetSelectedEntity(context.entity);
                            return true;
                        }
                    }
                    return false;
                };

                if (!checkComps(dropped->GetComponents()))
                {
                    checkComps(dropped->GetUnvalidatedComponents());
                }
            }
        }

        ImGui::EndDragDropTarget();
    }
}