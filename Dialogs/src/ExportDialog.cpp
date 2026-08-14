#include "ExportDialog.h"

#include "ProjectExporter.h"
#include "EventSystem.h"
#include "SceneManager.h"
#include "ColorPalletes.h"
#include "StatusManager.h"
#include "DialogManager.h"
#include "imgui.h"
#include <cstring>

REGISTER_DIALOG(ExportDialog);

ExportDialog::ExportDialog()
{
    std::strncpy(gameNameBuffer, gameName.c_str(), sizeof(gameNameBuffer) - 1);
    std::strncpy(executableNameBuffer, executableName.c_str(), sizeof(executableNameBuffer) - 1);
    std::strncpy(outputDirectoryBuffer, outputDirectory.c_str(), sizeof(outputDirectoryBuffer) - 1);
    std::strncpy(startupSceneBuffer, startupScene.c_str(), sizeof(startupSceneBuffer) - 1);

    EventSystem::get().Subscribe<OpenExportDialogEvent>(
        [this](const OpenExportDialogEvent&)
        {
            Open();
        });
}

void ExportDialog::Open()
{
    showDialog = true;
}

void ExportDialog::Close()
{
    showDialog = false;
    exporting = false;
}

void ExportDialog::Draw()
{
    if (!showDialog)
        return;

    ImGui::SetNextWindowSize(
        ImVec2(500, 0),
        ImGuiCond_FirstUseEver);

    if (!ImGui::Begin(
            "Export Project",
            &showDialog))
    {
        ImGui::End();
        return;
    }

    if (!SceneManager::get().HasProjectRoot())
    {
        ImGui::TextColored(
            COLOR_WARNING,
            "No project is currently loaded.");

        ImGui::Spacing();

        if (ImGui::Button("Close", ImVec2(120, 28)))
            Close();

        ImGui::End();
        return;
    }

    // ------------------------------------------------------------
    // Project
    // ------------------------------------------------------------

    ImGui::TextColored(
        COLOR_TEXT_DIM,
        "PROJECT");

    ImGui::Separator();

    ImGui::TextWrapped(
        "%s",
        SceneManager::get()
            .GetProjectRoot()
            .c_str());

    ImGui::Spacing();

    // ------------------------------------------------------------
    // Game name
    // ------------------------------------------------------------

    ImGui::TextColored(
        COLOR_TEXT_DIM,
        "GAME NAME");

    ImGui::SetNextItemWidth(-1);

ImGui::InputText("##gameName",gameNameBuffer,sizeof(gameNameBuffer));


    ImGui::Spacing();

    // ------------------------------------------------------------
    // Executable name
    // ------------------------------------------------------------

    ImGui::TextColored(
        COLOR_TEXT_DIM,
        "EXECUTABLE NAME");

    ImGui::SetNextItemWidth(-1);

    ImGui::InputText(
        "##executableName",
        executableNameBuffer,
        sizeof(executableNameBuffer));

    ImGui::Spacing();

    // ------------------------------------------------------------
    // Output directory
    // ------------------------------------------------------------

    ImGui::TextColored(
        COLOR_TEXT_DIM,
        "OUTPUT DIRECTORY");

    ImGui::SetNextItemWidth(-1);

    ImGui::InputText(
        "##outputDirectory",
        outputDirectoryBuffer,
        sizeof(outputDirectoryBuffer));

    ImGui::Spacing();

    // ------------------------------------------------------------
    // Startup scene
    // ------------------------------------------------------------

    ImGui::TextColored(
        COLOR_TEXT_DIM,
        "STARTUP SCENE");

    ImGui::SetNextItemWidth(-1);

    ImGui::InputText(
        "##startupScene",
        startupSceneBuffer,
        sizeof(startupSceneBuffer));

    ImGui::Spacing();

    // ------------------------------------------------------------
    // Options
    // ------------------------------------------------------------

    ImGui::TextColored(
        COLOR_TEXT_DIM,
        "OPTIONS");

    ImGui::Separator();

    ImGui::Checkbox(
        "Copy Assets",
        &copyAssets);

    ImGui::Checkbox(
        "Copy OpenAL",
        &copyOpenAL);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // ------------------------------------------------------------
    // Export
    // ------------------------------------------------------------

    if (exporting)
    {
        ImGui::TextColored(
            COLOR_WARNING,
            "Exporting...");

        ImGui::Spacing();
    }

    ImGui::BeginDisabled(exporting);

    if (ImGui::Button(
            "Export",
            ImVec2(150, 30)))
    {
        ProjectExporter::Settings settings;

        settings.gameName = gameNameBuffer;
        settings.executableName = executableNameBuffer;
        settings.outputDirectory = outputDirectoryBuffer;
        settings.startupScene = startupSceneBuffer;
        settings.copyAssets = copyAssets;
        settings.copyOpenAL = copyOpenAL;

        exporting = true;

        const bool success =
            ProjectExporter::Export(
                SceneManager::get().GetProjectRootPath(),
                settings);

        exporting = false;

        if (success)
        {
            StatusManager::get().Notify(
                "Project exported successfully",
                COLOR_SUCCESS);

            Close();
        }
        else
        {
            StatusManager::get().Notify(
                "Project export failed",
                COLOR_DANGER);
        }
    }

    ImGui::EndDisabled();

    ImGui::SameLine();

    if (ImGui::Button(
            "Cancel",
            ImVec2(150, 30)))
    {
        Close();
    }

    ImGui::End();
}