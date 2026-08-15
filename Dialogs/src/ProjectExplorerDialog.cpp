#include "ProjectExplorerDialog.h"

#include "ProjectExplorerManager.h"
#include "SceneManager.h"
#include "DialogManager.h"
#include "EventSystem.h"
#include "ColorPalletes.h"

#include "imgui.h"

#include <cstring>
#include <filesystem>
#include <vector>

REGISTER_DIALOG(ProjectExplorerDialog);

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

    DrawToolbar();

    // ---------------------------------------------------------
    // No project loaded
    // ---------------------------------------------------------

    if (!SceneManager::get().HasProjectRoot())
    {
        ImGui::Separator();

        ImGui::TextColored(
            COLOR_TEXT_DIM,
            "PROJECT ROOT");

        ImGui::TextColored(
            COLOR_WARNING,
            "No project selected");

        ImGui::Spacing();

        ImGui::TextWrapped(
            "Open or create a project to browse assets and scenes.");

        DrawProjectCreationPopup();

        ImGui::End();
        return;
    }

    // ---------------------------------------------------------
    // Project information
    // ---------------------------------------------------------

    ImGui::Separator();

    ImGui::TextColored(
        COLOR_TEXT_DIM,
        "PROJECT ROOT");

    ImGui::TextWrapped(
        "%s",
        SceneManager::get().GetProjectRoot().c_str());

    ImGui::Spacing();

    DrawProjectCreationPopup();

    // ---------------------------------------------------------
    // Assets
    // ---------------------------------------------------------

    DrawAssets();

    // ---------------------------------------------------------
    // Actions
    // ---------------------------------------------------------

    DrawActions();

    ImGui::End();
}

// =============================================================
// Toolbar
// =============================================================

void ProjectExplorerDialog::DrawToolbar()
{
    if (ImGui::Button(
            "New Project",
            ImVec2(100, 26)))
    {
        showNewProjectDialog = true;
    }

    ImGui::SameLine();

    if (ImGui::Button(
            SceneManager::get().HasProjectRoot()
                ? "Refresh"
                : "Open Project",
            ImVec2(90, 26)))
    {
        if (SceneManager::get().HasProjectRoot())
        {
            ProjectExplorerManager::get().SetDirectory(
                SceneManager::get().GetProjectRootPath());
        }
        else
        {
            EventSystem::get().Fire(
                OpenProjectLoadDialogEvent{});
        }
    }
}

// =============================================================
// Create Project
// =============================================================

void ProjectExplorerDialog::DrawProjectCreationPopup()
{
    if (showNewProjectDialog)
    {
        ImGui::OpenPopup("Create New Project");
        showNewProjectDialog = false;
    }

    if (!ImGui::BeginPopupModal(
            "Create New Project",
            nullptr,
            ImGuiWindowFlags_AlwaysAutoResize))
    {
        return;
    }

    ImGui::Text(
        "Choose a project name:");

    char buffer[128] = {0};

    std::strncpy(
        buffer,
        newProjectNameBuffer.c_str(),
        sizeof(buffer) - 1);

    if (ImGui::InputText(
            "##newProjectName",
            buffer,
            sizeof(buffer)))
    {
        newProjectNameBuffer = buffer;
    }

    if (ImGui::Button(
            "Create",
            ImVec2(120, 0)))
    {
        if (SceneManager::get().CreateNewProject(
                newProjectNameBuffer))
        {
            ProjectExplorerManager::get().SetDirectory(
                SceneManager::get().GetProjectRootPath());

            newProjectNameBuffer = "MyProject";

            ImGui::CloseCurrentPopup();
        }
    }

    ImGui::SameLine();

    if (ImGui::Button(
            "Cancel",
            ImVec2(120, 0)))
    {
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

// =============================================================
// Assets
// =============================================================

void ProjectExplorerDialog::DrawAssets()
{
    ImGui::Spacing();

    ImGui::TextColored(
        COLOR_TEXT_DIM,
        "ASSETS");

    ImGui::Separator();

    auto& explorer =
        ProjectExplorerManager::get();

    const std::filesystem::path projectRoot =
        SceneManager::get().GetProjectRootPath();

    // ---------------------------------------------------------
    // Make sure the explorer starts at the project root
    // ---------------------------------------------------------

    if (explorer.GetDirectory().empty())
    {
        explorer.SetDirectory(projectRoot);
    }

    std::filesystem::path currentDirectory =
        explorer.GetDirectory();

    // ---------------------------------------------------------
    // Safety check
    // ---------------------------------------------------------

    if (!std::filesystem::exists(currentDirectory))
    {
        explorer.SetDirectory(projectRoot);
        currentDirectory = projectRoot;
    }

    // ---------------------------------------------------------
    // Parent directory
    // ---------------------------------------------------------

    if (currentDirectory != projectRoot)
    {
        if (ImGui::Button(
                "..",
                ImVec2(-1, 24)))
        {
            const auto parent =
                currentDirectory.parent_path();

            // Don't allow navigation outside
            // the project root.
            if (!parent.empty() &&
                parent.string().find(
                    projectRoot.string()) == 0)
            {
                explorer.SetDirectory(parent);
            }
        }
    }

    // ---------------------------------------------------------
    // Directory contents
    // ---------------------------------------------------------

    const auto entries =
        explorer.GetEntries();

    for (const auto& entry : entries)
    {
        const auto path = entry.path();

        const std::string name =
            path.filename().string();

        // -----------------------------------------------------
        // Directories
        // -----------------------------------------------------

        if (entry.is_directory())
        {
            // Don't expose generated/build directories.
            if (name == "Generated" ||
                name == "build")
            {
                continue;
            }

            if (ImGui::Button(
                    ("[DIR] " + name).c_str(),
                    ImVec2(-1, 24)))
            {
                explorer.SetDirectory(path);
            }

            continue;
        }

        // -----------------------------------------------------
        // Files
        // -----------------------------------------------------

        FileType* fileType =
            explorer.GetFileType(path);

        if (fileType)
        {
            fileType->Draw(path);
        }
        else
        {
            DrawUnknownFile(path);
        }
    }
}

// =============================================================
// Unknown files
// =============================================================

void ProjectExplorerDialog::DrawUnknownFile(
    const std::filesystem::path& path)
{
    ImGui::Button(
        path.filename().string().c_str(),
        ImVec2(-1, 24));
}

// =============================================================
// Actions
// =============================================================

void ProjectExplorerDialog::DrawActions()
{
    ImGui::Spacing();

    ImGui::TextColored(
        COLOR_TEXT_DIM,
        "ACTIONS");

    ImGui::Separator();

    if (ImGui::Button(
            "New Scene",
            ImVec2(-1, 28)))
    {
        auto& explorer =
            ProjectExplorerManager::get();

        std::filesystem::path baseDir =
            explorer.GetDirectory();

        if (baseDir.empty())
        {
            baseDir =
                SceneManager::get().GetProjectRootPath();
        }

        std::string sceneName =
            "NewScene";

        std::filesystem::path scenePath =
            baseDir / (sceneName + ".scene");

        int suffix = 1;

        while (std::filesystem::exists(scenePath))
        {
            scenePath =
                baseDir /
                (sceneName +
                 std::to_string(suffix++) +
                 ".scene");
        }

        SceneManager::get().SaveSceneAs(
            scenePath.stem().string());

        explorer.SetDirectory(baseDir);
    }
}