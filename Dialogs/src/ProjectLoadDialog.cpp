#include "ProjectLoadDialog.h"
#include "DialogManager.h"
#include "EventSystem.h"
#include "SceneManager.h"
#include "imgui.h"
#include "Events.h"
#include <filesystem>
#include <algorithm>
#include "ColorPalletes.h"
REGISTER_DIALOG(ProjectLoadDialog);

ProjectLoadDialog::ProjectLoadDialog()
{
    openDialogSubscription = EventSystem::get().Subscribe<OpenProjectLoadDialogEvent>(
        [this](const OpenProjectLoadDialogEvent&)
        {
            showOpenProjectDialog = true;
            loadProjectPathError = false;
        });

    loadSuccessSubscription = EventSystem::get().Subscribe<ProjectLoadSuccessEvent>(
        [this](const ProjectLoadSuccessEvent&)
        {
            showOpenProjectDialog = false;
            loadProjectPathError = false;
            loadProjectPathBuffer.clear();
        });

    loadFailedSubscription = EventSystem::get().Subscribe<ProjectLoadFailedEvent>(
        [this](const ProjectLoadFailedEvent&)
        {
            showOpenProjectDialog = true;
            loadProjectPathError = true;
        });
}
void ProjectLoadDialog::Draw()
{
    if (!showOpenProjectDialog && SceneManager::get().HasProjectRoot())
        return;

    ImGui::SetNextWindowSize(ImVec2(520, 0), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("Open Project", &showOpenProjectDialog))
    {
        ImGui::End();
        return;
    }

    ImGui::TextColored(COLOR_TEXT_DIM, "DISCOVERED COLORS");
    ImGui::Separator();

    auto projects = SceneManager::get().GetAvailableProjects();

    if (projects.empty())
    {
        ImGui::TextColored(COLOR_TEXT_DIM, "No projects found under Projects/");
    }
    else
    {
        for (const auto& projectPath : projects)
        {
            std::string label = projectPath.filename().string();

            if (ImGui::Button(label.c_str(), ImVec2(-1, 26)))
                EventSystem::get().Fire(OpenProjectEvent{projectPath.string()});
        }
    }

    ImGui::Spacing();
    ImGui::TextColored(COLOR_TEXT_DIM, "OPEN BY PATH");
    ImGui::Separator();

    ImGui::SetNextItemWidth(-1);
    ImGui::InputText("##projectPath", &loadProjectPathBuffer[0], loadProjectPathBuffer.size() + 1);

    if (loadProjectPathError)
        ImGui::TextColored(COLOR_DANGER, "Project folder not found or invalid");

    if (ImGui::Button("Open Path", ImVec2(-1, 28)))
        EventSystem::get().Fire(OpenProjectEvent{loadProjectPathBuffer});

    if (!SceneManager::get().HasProjectRoot())
        ImGui::TextColored(COLOR_WARNING, "The editor is currently in no-project mode.");

    ImGui::End();
}

void ProjectLoadDialog::Open()
{
    showOpenProjectDialog = true;
    loadProjectPathError = false;
}

void ProjectLoadDialog::Close()
{
    showOpenProjectDialog = false;
}