#pragma once

#include "Dialog.h"
#include "Logger.h"
#include <deque>
#include <string>
#include <imgui.h>
#include "ConsoleManager.h"

class EntitiesDialog : public Dialog
{
public:
    EntitiesDialog();

    void Draw() override;
    void Open() override;
    void Close() override;
};