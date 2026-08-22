#include "ScriptFile.h"

#include "imgui.h"

bool ScriptFile::Supports(
    const std::filesystem::path& path) const
{
    return path.extension() == ".cpp" ||
           path.extension() == ".h";
}

void ScriptFile::Draw(
    const std::filesystem::path& path)
{
    const std::string name =
        path.filename().string();

    ImGui::Button(
        ("[SCRIPT] " + name).c_str(),
        ImVec2(-1, 24));

    if (ImGui::BeginDragDropSource())
    {
        const std::string absolutePath =
            std::filesystem::absolute(path)
                .lexically_normal()
                .string();

        ImGui::SetDragDropPayload(
            "SCRIPT",
            absolutePath.c_str(),
            absolutePath.size() + 1);

        ImGui::Text("%s", name.c_str());

        ImGui::EndDragDropSource();
    }
}