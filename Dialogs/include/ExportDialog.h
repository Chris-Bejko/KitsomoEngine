#pragma once

#include "Dialog.h"
#include <string>

class ExportDialog : public Dialog
{
public:
    ExportDialog();

    void Draw() override;
    void Open() override;
    void Close() override;

private:
    bool showDialog = false;

    std::string gameName = "MyGame";
    std::string executableName = "MyGame";
    std::string outputDirectory = "Build";
    std::string startupScene = "MainNew";

    char gameNameBuffer[256]{};
    char executableNameBuffer[256]{};
    char outputDirectoryBuffer[512]{};
    char startupSceneBuffer[256]{};
    
    bool copyAssets = true;
    bool copyOpenAL = true;

    bool exporting = false;
};