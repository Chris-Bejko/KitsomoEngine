#include "ProjectExplorerDialog.h"
#include "SceneManager.h"
#include "imgui.h"
#include "ColorPalletes.h"

#include <algorithm>
#include <cstring>
#include <vector>
#include "DialogManager.h"
REGISTER_DIALOG(ProjectExplorerDialog);
namespace
{
	bool IsTextureFile(const std::string &name)
	{
		std::string extension = std::filesystem::path(name).extension().string();
		std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c)
					   { return static_cast<char>(std::tolower(c)); });
		return extension == ".png" || extension == ".jpg" || extension == ".jpeg" || extension == ".bmp" || extension == ".tga" || extension == ".gif";
	}
}
ProjectExplorerDialog::ProjectExplorerDialog()
{
    SetName("Project Explorer");
}

void ProjectExplorerDialog::Open()
{
}

void ProjectExplorerDialog::Close()
{
}

void ProjectExplorerDialog::Draw()
{
    ImGui::Begin("Project Explorer");

    if (ImGui::Button("New Project", ImVec2(100, 26)))
        showNewProjectDialog = true;

    ImGui::SameLine();

    if (ImGui::Button(SceneManager::get().HasProjectRoot() ? "Refresh" : "Open Project", ImVec2(90, 26)))
    {
        if (SceneManager::get().HasProjectRoot())
            projectExplorerDirectory = std::filesystem::path();
        else
            ; // Open project dialog will be moved here later
    }

    if (!SceneManager::get().HasProjectRoot())
    {
        ImGui::Separator();
        ImGui::TextColored(COLOR_TEXT_DIM, "PROJECT ROOT");
        ImGui::TextColored(COLOR_WARNING, "No project selected");
        ImGui::Spacing();
        ImGui::TextWrapped("Open or create a project to browse assets and scenes.");

        DrawProjectCreationPopup();

        ImGui::End();
        return;
    }

    ImGui::Separator();
    ImGui::TextColored(COLOR_TEXT_DIM, "PROJECT ROOT");
    ImGui::TextWrapped("%s", SceneManager::get().GetProjectRoot().c_str());
    ImGui::Spacing();

    DrawProjectCreationPopup();
    DrawAssets();
    DrawActions();

    ImGui::End();
}

void ProjectExplorerDialog::DrawProjectCreationPopup()
{
    if (showNewProjectDialog)
    {
        ImGui::OpenPopup("Create New Project");
        showNewProjectDialog = false;
    }

    if (!ImGui::BeginPopupModal("Create New Project", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        return;

    ImGui::Text("Choose a project name:");

    char buffer[128] = {0};
    std::strncpy(buffer, newProjectNameBuffer.c_str(), sizeof(buffer) - 1);

    if (ImGui::InputText("##newProjectName", buffer, sizeof(buffer)))
        newProjectNameBuffer = buffer;

    if (ImGui::Button("Create", ImVec2(120, 0)))
    {
        if (SceneManager::get().CreateNewProject(newProjectNameBuffer))
        {
            projectExplorerDirectory = std::filesystem::path();
            newProjectNameBuffer = "MyProject";
            ImGui::CloseCurrentPopup();
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Cancel", ImVec2(120, 0)))
        ImGui::CloseCurrentPopup();

    ImGui::EndPopup();
}

void ProjectExplorerDialog::DrawAssets()
{
    ImGui::Spacing();
    ImGui::TextColored(COLOR_TEXT_DIM, "ASSETS");
    ImGui::Separator();

    std::filesystem::path baseDir =
        projectExplorerDirectory.empty()
            ? SceneManager::get().GetProjectRootPath()
            : projectExplorerDirectory;

    if (!std::filesystem::exists(baseDir))
        baseDir = SceneManager::get().GetProjectRootPath();

    if (!projectExplorerDirectory.empty())
    {
        if (ImGui::Button("..", ImVec2(-1, 24)))
            projectExplorerDirectory = baseDir.parent_path();
    }

    std::vector<std::filesystem::directory_entry> entries;

    for (const auto& entry : std::filesystem::directory_iterator(baseDir))
        entries.push_back(entry);

    std::sort(entries.begin(), entries.end(),
        [](const auto& a, const auto& b)
        {
            return a.path().filename().string() < b.path().filename().string();
        });

    for (const auto& entry : entries)
    {
        const auto path = entry.path();
        const std::string name = path.filename().string();
        const bool isDirectory = std::filesystem::is_directory(path);
        const bool shouldIgnore = name == "Generated" || name == "build";

       	if (isDirectory && !shouldIgnore)
		{
			if (ImGui::Button(("[DIR] " + name).c_str(), ImVec2(-1, 24)))
				projectExplorerDirectory = path;
		}
		else
		{
			const std::string ext = path.extension().string();
			if (ext == ".scene")
			{
				if (ImGui::Button(("[SCENE] " + name).c_str(), ImVec2(-1, 24)))
					SceneManager::get().LoadScene(path.stem().string(), SceneLoadMode::Replace);
			}
			else if (ext == ".prefab")
			{
				ImGui::Button(("[PREFAB] " + name).c_str(), ImVec2(-1, 24));
			}
			else if (!isDirectory)
			{
				if (IsTextureFile(name))
				{
					ImGui::Button(name.c_str(), ImVec2(-1, 24));

					if (ImGui::BeginDragDropSource())
					{
						const std::string absolutePath = std::filesystem::relative(path).string();
						ImGui::SetDragDropPayload("ASSET_PATH", absolutePath.c_str(), absolutePath.size() + 1);
						ImGui::Text("%s", name.c_str());
						ImGui::EndDragDropSource();
					}
				}
				else
					ImGui::Button(name.c_str(), ImVec2(-1, 24));
			}
		}
	}
}

void ProjectExplorerDialog::DrawActions()
{
    ImGui::Spacing();
    ImGui::TextColored(COLOR_TEXT_DIM, "ACTIONS");
    ImGui::Separator();

    if (ImGui::Button("New Scene", ImVec2(-1, 28)))
    {
        std::filesystem::path baseDir =
            projectExplorerDirectory.empty()
                ? SceneManager::get().GetProjectRootPath()
                : projectExplorerDirectory;

        std::string sceneName = "NewScene";
        std::filesystem::path scenePath = baseDir / (sceneName + ".scene");

        int suffix = 1;

        while (std::filesystem::exists(scenePath))
            scenePath = baseDir / (sceneName + std::to_string(suffix++) + ".scene");

        SceneManager::get().SaveSceneAs(scenePath.stem().string());
        projectExplorerDirectory = baseDir;
    }
}