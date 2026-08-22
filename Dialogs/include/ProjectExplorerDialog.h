#pragma once

#include "Dialog.h"

#include <filesystem>
#include <string>

class ProjectExplorerDialog : public Dialog
{
public:
    ProjectExplorerDialog();

    void Draw() override;
    void Open() override;
    void Close() override;

private:
    void DrawToolbar();
    void DrawProjectCreationPopup();
    void DrawAssets();
    void DrawActions();
    void DrawUnknownFile(
        const std::filesystem::path& path);

    bool showNewProjectDialog = false;

    std::string newProjectNameBuffer =
        "MyProject";
};