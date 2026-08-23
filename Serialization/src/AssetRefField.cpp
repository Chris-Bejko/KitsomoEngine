#include "AssetRefField.h"

#include "imgui.h"
#include <filesystem>
#include "SerializableScript.h"

void AssetRefField::Draw(const FieldDrawContext &context)
{
    if (!asset)
        return;

    const std::string &path = asset->GetPath();

    std::string displayName = path.empty() ? "None (drop asset here)" : std::filesystem::path(path).filename().string();

    ImVec4 boxColor = path.empty() ? ImVec4(0.15f, 0.18f, 0.25f, 1.0f) : ImVec4(0.1f, 0.25f, 0.1f, 1.0f);

    ImGui::PushStyleColor(ImGuiCol_Button, boxColor);
    ImGui::Text("%s", name);

    ImGui::Button((displayName + "##" + context.fieldId).c_str(), ImVec2(-1, 24));

    ImGui::PopStyleColor();

    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("ASSET_PATH"))
        {
            const char *droppedPath = static_cast<const char *>(payload->Data);

            if (droppedPath)
            {
                asset->SetPath(droppedPath);

                if (context.script)
                    context.script->NotifyFieldChanged(GetName());
            }
        }

        ImGui::EndDragDropTarget();
    }

    if (ImGui::BeginPopupContextItem(("##ctx" + context.fieldId).c_str()))
    {
        if (ImGui::MenuItem("Clear"))
        {
            asset->SetPath("");

            if (context.script)
                context.script->NotifyFieldChanged(GetName());
        }

        ImGui::EndPopup();
    }
}

std::string AssetRefField::Serialize() 
{
    if (!asset)
        return "";

    return asset->GetPath();
}

void AssetRefField::Deserialize(const std::string& serialized)
{
    if (!asset)
        return;

    asset->SetPath(serialized);
}
