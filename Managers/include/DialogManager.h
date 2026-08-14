#pragma once

#include <memory>
#include <string>
#include <vector>

class Dialog;

class DialogManager
{
public:
    static DialogManager& get();

    void Update();
    void AddDialog(std::unique_ptr<Dialog> dialog);

private:
    DialogManager() = default;
    ~DialogManager() = default;

    DialogManager(const DialogManager&) = delete;
    DialogManager& operator=(const DialogManager&) = delete;

    std::vector<std::unique_ptr<Dialog>> dialogs;
};

#define REGISTER_DIALOG(Type) \
    static bool _registered_dialog_##Type = []() \
    { \
        auto dialog = std::make_unique<Type>(); \
        dialog->SetName(#Type); \
        DialogManager::get().AddDialog(std::move(dialog)); \
        return true; \
    }()