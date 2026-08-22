#include "TextureFile.h"

#include "imgui.h"

bool TextureFile::Supports(
    const std::filesystem::path &path) const
{
    const std::string extension =
        path.extension().string();

    return extension == ".png" ||
           extension == ".jpg" ||
           extension == ".jpeg" ||
           extension == ".bmp" ||
           extension == ".tga";
}

void TextureFile::Draw(const std::filesystem::path &path)
{
    const std::string name =
        path.filename().string();

    ImGui::Button(
        ("[TEXTURE] " + name).c_str(),
        ImVec2(-1, 24));

    if (ImGui::BeginDragDropSource())
    {
        const std::string absolutePath = std::filesystem::absolute(path).lexically_normal().string();

        ImGui::SetDragDropPayload("ASSET_PATH", absolutePath.c_str(), absolutePath.size() + 1);

        ImGui::Text("%s", name.c_str());

        ImGui::EndDragDropSource();
    }
}