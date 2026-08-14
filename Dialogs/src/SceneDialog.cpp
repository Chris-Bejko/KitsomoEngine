#include "SceneDialog.h"
#include "SceneManager.h"
#include "imgui.h"
#include "DialogManager.h"
REGISTER_DIALOG(SceneDialog);

static const ImVec4 COLOR_ACCENT = ImVec4(0.25f, 0.52f, 0.95f, 1.0f);
static const ImVec4 COLOR_SUCCESS = ImVec4(0.18f, 0.65f, 0.35f, 1.0f);
static const ImVec4 COLOR_DANGER = ImVec4(0.75f, 0.18f, 0.18f, 1.0f);
static const ImVec4 COLOR_TEXT_DIM = ImVec4(0.55f, 0.55f, 0.60f, 1.0f);

SceneDialog::SceneDialog()
{
    SetName("Scenes");
}

void SceneDialog::Open()
{
}

void SceneDialog::Close()
{
}

void SceneDialog::Draw()
{
    if (!SceneManager::get().HasProjectRoot())
        return;

    ImGui::Begin("Scenes");

    const std::string current = SceneManager::get().GetCurrentScene();

    ImGui::TextColored(COLOR_TEXT_DIM, "CURRENT SCENE");

    if (current.empty())
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Unsaved scene");
    else
        ImGui::TextColored(COLOR_ACCENT, "%s", current.c_str());

    ImGui::Separator();
    ImGui::Spacing();

    ImGui::PushStyleColor(ImGuiCol_Button, COLOR_SUCCESS);
    if (ImGui::Button("Save Scene", ImVec2(-1, 28)))
    {
        if (current.empty())
            saveScenePressed = true;
        else
            SceneManager::get().SaveCurrentScene();
    }
    ImGui::PopStyleColor();

    ImGui::PushStyleColor(ImGuiCol_Button, COLOR_ACCENT);
    if (ImGui::Button("Save Scene As...", ImVec2(-1, 28)))
        saveScenePressed = true;
    ImGui::PopStyleColor();

    DrawSaveSceneDialog();

    ImGui::Spacing();
    ImGui::TextColored(COLOR_TEXT_DIM, "AVAILABLE SCENES");
    ImGui::Separator();
    ImGui::Spacing();

    auto scenes = SceneManager::get().GetAvailableScenes();

    if (scenes.empty())
    {
        ImGui::TextColored(COLOR_TEXT_DIM, "No scenes found");
    }
    else
    {
        for (const auto& scene : scenes)
        {
            bool isLoaded = scene == current;

            if (isLoaded)
                ImGui::PushStyleColor(ImGuiCol_Button, COLOR_ACCENT);
            else
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.22f, 0.35f, 1.0f));

            std::string label = (isLoaded ? "* " : "  ") + scene + "##scene";

            if (ImGui::Button(label.c_str(), ImVec2(-1, 24)))
            {
                selectedScene = scene;
                showLoadOptions = true;
            }

            ImGui::PopStyleColor();
        }
    }

    DrawLoadOptions();

    ImGui::End();
}

void SceneDialog::DrawSaveSceneDialog()
{
    if (!saveScenePressed)
        return;

    ImGui::SetNextWindowSize(ImVec2(300, 0), ImGuiCond_Always);
    ImGui::Begin("Save Scene As", &saveScenePressed);

    ImGui::SetNextItemWidth(-1);
    ImGui::InputText("##scenename", &sceneNameBuffer[0], 128);

    ImGui::PushStyleColor(ImGuiCol_Button, COLOR_SUCCESS);

    if (ImGui::Button("Save", ImVec2(-1, 28)))
    {
        SceneManager::get().SaveSceneAs(std::string(sceneNameBuffer.c_str()));
        saveScenePressed = false;
    }

    ImGui::PopStyleColor();
    ImGui::End();
}


void SceneDialog::DrawLoadOptions()
{
    if (!showLoadOptions || selectedScene.empty())
        return;

    ImGui::SetNextWindowSize(ImVec2(250, 0), ImGuiCond_Always);
    ImGui::Begin(("Load: " + selectedScene).c_str(), &showLoadOptions);

    ImGui::PushStyleColor(ImGuiCol_Button, COLOR_DANGER);

    if (ImGui::Button("Replace (clear current)", ImVec2(-1, 28)))
    {
        SceneManager::get().LoadScene(selectedScene, SceneLoadMode::Replace);
        showLoadOptions = false;
    }

    ImGui::PopStyleColor();

    ImGui::PushStyleColor(ImGuiCol_Button, COLOR_ACCENT);

    if (ImGui::Button("Additive (keep current)", ImVec2(-1, 28)))
    {
        SceneManager::get().LoadScene(selectedScene, SceneLoadMode::Additive);
        showLoadOptions = false;
    }

    ImGui::PopStyleColor();
    ImGui::End();
}