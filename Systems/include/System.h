#pragma once

class System
{
public:
    System() = default;
    virtual ~System() = default;

    virtual bool Init() { return true; }
    virtual void Update() {}
};
