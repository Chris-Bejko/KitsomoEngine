#pragma once

#include "Dialog.h"

class ToolbarDialog : public Dialog
{
public:
    ToolbarDialog();

    void Draw() override;
    void Open() override;
    void Close() override;
private:
    void OnPlay();
    void OnPause();
    void OnReset();
    void DrawScriptStatus();
};