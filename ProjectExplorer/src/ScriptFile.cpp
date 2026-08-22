#include "ScriptFile.h"

#include "imgui.h"

bool ScriptFile::Supports(const std::filesystem::path &path) const
{
    return path.extension() == ".cpp" ||
           path.extension() == ".h";
}

void ScriptFile::Draw(const std::filesystem::path &path)
{
    const std::string name = path.filename().string();

    ImGui::Button(("[SCRIPT] " + name).c_str(), ImVec2(-1, 24));

    if (ImGui::BeginDragDropSource())
    {
        std::string componentName = path.stem().string();

        ImGui::SetDragDropPayload("SCRIPT", componentName.c_str(), componentName.size() + 1);

        ImGui::Text("%s", name.c_str());

        ImGui::EndDragDropSource();
    }
}