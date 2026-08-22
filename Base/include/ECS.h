#pragma once

#include <array>
#include <bitset>
#include <cstddef>
#include <cstdint>

class Component;

using ComponentID = std::uint32_t;

constexpr ComponentID INVALID_COMPONENT_ID = static_cast<ComponentID>(-1);

constexpr std::size_t maxComponents = 128;

struct ComponentList : std::array<Component *, maxComponents>
{
    ComponentList()
    {
        fill(nullptr);
    }
};

using ComponentBitset = std::bitset<maxComponents>;

template <typename T>
ComponentID getComponentTypeID() noexcept;