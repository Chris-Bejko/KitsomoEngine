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
    std::filesystem::path projectExplorerDirectory;
    bool showNewProjectDialog = false;
    std::string newProjectNameBuffer = "MyProject";

    void DrawProjectCreationPopup();
    void DrawAssets();
    void DrawActions();
};