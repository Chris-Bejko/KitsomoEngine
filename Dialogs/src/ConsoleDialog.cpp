#include "ConsoleDialog.h"

static const ImVec4 COLOR_DANGER = ImVec4(0.75f, 0.18f, 0.18f, 1.0f);
static const ImVec4 COLOR_TEXT_DIM = ImVec4(0.55f, 0.55f, 0.60f, 1.0f);

namespace
{
	ImVec4 GetValueColor(LogLevel level)
	{
		switch (level)
		{
		case LogLevel::Info:
			return ImVec4(0.9f, 0.9f, 0.9f, 1.0f);
		case LogLevel::Warning:
			return ImVec4(0.95f, 0.78f, 0.2f, 1.0f);
		case LogLevel::Error:
			return ImVec4(1.0f, 0.3f, 0.3f, 1.0f);
		case LogLevel::Debug:
			return ImVec4(0.4f, 0.85f, 1.0f, 1.0f);
		default:
			return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		}
	}
}

ConsoleDialog::ConsoleDialog()
	: consoleManager(ConsoleManager::get())
{
	SetName("Console");
}

void ConsoleDialog::Open()
{
}

void ConsoleDialog::Close()
{
}

void ConsoleDialog::Draw()
{
	ImGui::Begin("Console");

	ImGui::PushStyleColor(ImGuiCol_Button, COLOR_DANGER);

	if (ImGui::Button("Clear", ImVec2(70, 24)))
		consoleManager.Clear();

	ImGui::PopStyleColor();

	ImGui::SameLine();

	const auto &logs = consoleManager.GetConsoleLogs();
	ImGui::TextColored(COLOR_TEXT_DIM, "%zu entries", logs.size());

	ImGui::Separator();
	ImGui::Spacing();

	ImGui::BeginChild("##consolescroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

	for (const auto &entry : logs)
		ImGui::TextColored(GetValueColor(entry.level), "%s", entry.message.c_str());

	if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
		ImGui::SetScrollHereY(1.0f);

	ImGui::EndChild();
	ImGui::End();
}

