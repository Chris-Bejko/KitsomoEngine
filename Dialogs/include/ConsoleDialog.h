#pragma once

#include "Dialog.h"
#include "Logger.h"
#include <deque>
#include <string>
#include <imgui.h>
#include "ConsoleManager.h"

class ConsoleDialog : public Dialog
{
public:
    ConsoleDialog();

    void Draw() override;
    void Open() override;
    void Close() override;
private:
    ImVec4 GetLogColor(LogLevel level) const;
    ConsoleManager& consoleManager;
};