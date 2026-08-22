#include "PrefabFile.h"

#include "imgui.h"

bool PrefabFile::Supports(const std::filesystem::path &path) const
{
    return path.extension() == ".prefab";
}

void PrefabFile::Draw(const std::filesystem::path &path)
{
    const std::string name = path.filename().string();

    ImGui::Button(("[PREFAB] " + name).c_str(), ImVec2(-1, 24));

    if (ImGui::BeginDragDropSource())
    {
        const std::string absolutePath = std::filesystem::absolute(path).lexically_normal().string();

        ImGui::SetDragDropPayload("PREFAB", absolutePath.c_str(), absolutePath.size() + 1);

        ImGui::Text("%s", name.c_str());

        ImGui::EndDragDropSource();
    }
}