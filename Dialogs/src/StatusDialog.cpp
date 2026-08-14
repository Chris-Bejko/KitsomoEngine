#include "StatusDialog.h"
#include "imgui.h"
#include <algorithm>
#include "DialogManager.h"
#include "ColorPalletes.h"

REGISTER_DIALOG(StatusDialog);

StatusDialog::StatusDialog()
    : statusManager(StatusManager::get())
{
    SetName("Status");
}

void StatusDialog::Open()
{
}

void StatusDialog::Close()
{
}

void StatusDialog::Draw()
{
    const auto& statuses = statusManager.GetStatuses();

    if (statuses.empty())
        return;

    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.0f);
    ImGui::Begin("##status", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysAutoResize);

    for (const auto& status : statuses)
    {
        float alpha = std::min(1.0f, status.lifetime / 0.5f);
        ImVec4 color = status.color;
        color.w = alpha;

        float progress = status.lifetime / status.maxLifetime;

        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(color.x, color.y, color.z, 0.4f));
        ImGui::ProgressBar(progress, ImVec2(-1, 3), "");
        ImGui::PopStyleColor();

        ImGui::TextColored(color, "%s", status.message.c_str());
        ImGui::Spacing();
    }

    ImGui::End();
}