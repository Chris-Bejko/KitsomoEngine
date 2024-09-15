#pragma once

#include <array>
#include <bitset>
#include <iostream>


class Entity;
class Component;


using ComponentTypeID = std::size_t;

inline ComponentTypeID getUniqueComponentID()
{
    static ComponentTypeID lastID = 0u;
    return lastID++;
}

template<typename T>
inline ComponentTypeID getComponentTypeID() noexcept
{
    static_assert(std::is_base_of<Component, T>::value, "Type not base of component.");
    static const ComponentTypeID typeID = getUniqueComponentID();
    return typeID;
}

constexpr std::size_t MAX_ENTITIES = 5000;
constexpr std::size_t MAX_COMPONENTS = 32;

using ComponentBitset = std::bitset<MAX_COMPONENTS>;
using ComponentList = std::array<Component*, MAX_COMPONENTS>;

