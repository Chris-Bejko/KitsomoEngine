#include "DialogManager.h"
#include "Dialogs/include/base/Dialog.h"
#include "Logger.h"
#include <algorithm>

DialogManager& DialogManager::get()
{
    static DialogManager instance;
    return instance;
}

void DialogManager::Update()
{
    for (auto& dialog : dialogs)
        dialog->Draw();
}

void DialogManager::AddDialog(std::unique_ptr<Dialog> dialog)
{
    const std::string& name = dialog->GetName();

    if (std::any_of(dialogs.begin(), dialogs.end(), [&](const auto& d) { return d->GetName() == name; }))
    {
        LOG_WARNING("Dialog with name '", name, "' already exists.");
        return;
    }

    dialogs.push_back(std::move(dialog));
    LOG_INFO("Registered dialog: ", name);
}