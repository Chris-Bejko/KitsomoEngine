#pragma once

#include "SystemManager.h"
#include "Dialogs/include/base/Dialog.h"


class DialogSystem : public System
{
    public:
        DialogSystem() = default;
        virtual ~DialogSystem() = default;

        bool Init() override { return true; };
        void Update() override;
        
        void AddDialog(std::unique_ptr<Dialog> dialog, std::string name = "");
    private:
        std::vector<std::tuple<std::unique_ptr<Dialog>, std::string>> dialogs;

};