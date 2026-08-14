#include "ToolbarDialog.h"
#include "HotReloading/ScriptCompiler.h"
#include "imgui.h"
#include "ColorPalletes.h"
#include "SceneManager.h"
#include "Engine.h"
#include "EventSystem.h"
#include "GizmoSystem.h"
#include "DialogManager.h"
REGISTER_DIALOG(ToolbarDialog);

ToolbarDialog::ToolbarDialog()
{
}


void ToolbarDialog::Open()
{
}
void ToolbarDialog::Close()
{
}
void ToolbarDialog::Draw()
{
	ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_Always);
	ImGui::Begin("##toolbar", nullptr,
				 ImGuiWindowFlags_NoTitleBar |
					 ImGuiWindowFlags_NoResize |
					 ImGuiWindowFlags_NoScrollbar);

	auto stateButton = [](const char *label, ImVec4 color, bool &pressed)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, color);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(color.x + 0.1f, color.y + 0.1f, color.z + 0.1f, 1.0f));
		pressed = ImGui::Button(label, ImVec2(70, 28));
		ImGui::PopStyleColor(2);
		return pressed;
	};

	bool pressed = false;
	if (!SceneManager::get().HasProjectRoot())
	{
		if (stateButton("Open", COLOR_ACCENT, pressed))
			EventSystem::get().Fire(OpenProjectLoadDialogEvent{});
		ImGui::SameLine();
		ImGui::SameLine();
		ImGui::TextColored(COLOR_WARNING, "No project loaded");
		ImGui::End();
		return;
	}

	switch (Engine::get().GetCurrentState())
	{
	case EngineState::Running:
		if (stateButton("  Play", COLOR_SUCCESS, pressed))
			OnPlay();
		break;

	case EngineState::PlayMode:
		if (stateButton(" Pause", COLOR_WARNING, pressed))
			OnPause();
		ImGui::SameLine();
        if(stateButton(" Reset", COLOR_DANGER, pressed))
			OnReset();
		break;
	case EngineState::Paused:
		if (stateButton("  Play", COLOR_SUCCESS, pressed))
			OnPlay();
		break;
	}


	ImGui::SameLine();
	ImGui::Separator();
	ImGui::SameLine();

	auto gizmoButton = [](const char *label, GizmoMode btnMode)
	{
		bool active = GizmoSystem::get().GetMode() == btnMode;
		if (active)
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.52f, 0.95f, 1.0f));
		if (ImGui::Button(label, ImVec2(40, 28)))
			GizmoSystem::get().SetMode(btnMode);
		if (active)
			ImGui::PopStyleColor();
	};

	gizmoButton("W", GizmoMode::Move);
	ImGui::SameLine();
	gizmoButton("E", GizmoMode::Rotate);
	ImGui::SameLine();
	gizmoButton("R", GizmoMode::Scale);
	ImGui::SameLine();

	// Snap toggle
	bool snap = GizmoSystem::get().snapEnabled;
	if (snap)
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.85f, 0.60f, 0.10f, 1.0f));
	if (ImGui::Button("Snap", ImVec2(50, 28)))
		GizmoSystem::get().snapEnabled = !GizmoSystem::get().snapEnabled;
	if (snap)
		ImGui::PopStyleColor();

	ImGui::SameLine();
	DrawScriptStatus();
	ImGui::SameLine();
	if (ImGui::Button("Reload Scripts", ImVec2(110, 28)))
	{
		Engine::get().RequestScriptRecompile();
	}
	ImGui::End();
}


void ToolbarDialog::OnPlay()
{
	Engine::get().SetEngineState(EngineState::PlayMode);
}
void ToolbarDialog::OnPause() { Engine::get().SetEngineState(EngineState::Paused); }
void ToolbarDialog::OnReset()
{
	Engine::get().SetEngineState(EngineState::Running);
}

void ToolbarDialog::DrawScriptStatus()
{
	if (Engine::get().pendingRecompile)
	{
		ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f),
						   "● Recompiling...");
	}
	else if (ScriptCompiler::lastCompileFailed)
	{
		ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f),
						   "● Compile Error");
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("%s", ScriptCompiler::lastError.c_str());
	}
	else
	{
		ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f),
						   "● Scripts OK");
	}
}
