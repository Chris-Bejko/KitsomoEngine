#include "DialogSystem.h"
#include "Dialogs/include/base/Dialog.h"
#include "Logger.h"
#include <algorithm>

void DialogSystem::Update()
{
    for (auto& dialog : dialogs)
    {
        std::get<0>(dialog)->Draw();
    }
};


void DialogSystem::AddDialog(std::unique_ptr<Dialog> dialog, std::string name)
{
    if (std::any_of(dialogs.begin(), dialogs.end(), [&](const auto& d) { return std::get<1>(d) == name; }))
    {
        LOG_WARNING("Dialog with name '", name, "' already exists. Choose a new name for your dialog.");
        return;
    }
    dialogs.push_back(std::make_tuple(std::move(dialog), name));
};

