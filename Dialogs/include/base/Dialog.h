#pragma once
#include <string>

class Dialog
{
public:
    Dialog() = default;
    virtual ~Dialog() = default;

    virtual void Draw() = 0;
    virtual void Open() = 0;
    virtual void Close() = 0;
    std::string GetName() const { return name; }
    void SetName(const std::string& newName) { name = newName; }
private:
    std::string name;
};