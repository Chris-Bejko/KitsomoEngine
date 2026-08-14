#pragma once
#include "base/Dialog.h"

class InspectorDialog : public Dialog
{
public:
    InspectorDialog() = default;
    virtual ~InspectorDialog() = default;

    void Draw() override;
    void Open() override;
    void Close() override;
};