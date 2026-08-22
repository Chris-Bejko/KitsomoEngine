#include "FontFile.h"

#include "imgui.h"

bool FontFile::Supports(const std::filesystem::path &path) const
{
    const std::string extension = path.extension().string();

    return extension == ".ttf" ||
           extension == ".otf";
}

void FontFile::Draw(const std::filesystem::path &path)
{
    const std::string name = path.filename().string();

    ImGui::Button(("[FONT] " + name).c_str(), ImVec2(-1, 24));

    if (ImGui::BeginDragDropSource())
    {
        const std::string absolutePath = std::filesystem::absolute(path).lexically_normal().string();

        ImGui::SetDragDropPayload("FONT", absolutePath.c_str(), absolutePath.size() + 1);

        ImGui::Text("%s", name.c_str());

        ImGui::EndDragDropSource();
    }
}