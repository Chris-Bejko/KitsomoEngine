#pragma once

#include <array>
#include <bitset>
#include <cstddef>

class Component;

using ComponentID = std::size_t;

inline ComponentID getNewComponentTypeID()
{
    static ComponentID lastID = 0u;
    return lastID++;
}

template <typename T>
inline ComponentID getComponentTypeID() noexcept
{
    static ComponentID typeID = getNewComponentTypeID();
    return typeID;
}

constexpr std::size_t maxComponents = 128;

struct ComponentList : std::array<Component *, maxComponents>
{
    ComponentList()
    {
        fill(nullptr);
    }
};

using ComponentBitset = std::bitset<maxComponents>;