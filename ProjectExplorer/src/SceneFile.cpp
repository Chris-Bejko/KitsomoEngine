#include "SceneFile.h"

#include "imgui.h"
#include "SceneManager.h"

bool SceneFile::Supports(
    const std::filesystem::path& path) const
{
    return path.extension() == ".scene";
}

void SceneFile::Draw(
    const std::filesystem::path& path)
{
    const std::string name =
        path.filename().string();

    if (ImGui::Button(
            ("[SCENE] " + name).c_str(),
            ImVec2(-1, 24)))
    {
        SceneManager::get().LoadScene(
            path.stem().string(),
            SceneLoadMode::Replace);
    }
}