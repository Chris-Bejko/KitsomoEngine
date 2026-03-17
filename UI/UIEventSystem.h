#pragma once
#include <vector>
#include <set>

class UIButton;

class UIEventSystem
{
public:
    static UIEventSystem& get()
    {
        static UIEventSystem instance;
        return instance;
    }

    void Update();
    void Register(UIButton* button);
    void Unregister(UIButton* button);
    void Clear();

private:
    UIEventSystem() = default;
    std::vector<UIButton*> buttons;
    std::set<UIButton*> hoveredButtons;
};