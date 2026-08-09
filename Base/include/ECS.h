#pragma once

#include <array>
#include <bitset>
#include <cstddef>
#include <cstdint>

class Component;

// ============================================================
// Component IDs
// ============================================================

using ComponentID = std::uint32_t;

constexpr ComponentID INVALID_COMPONENT_ID =
    static_cast<ComponentID>(-1);


// ============================================================
// ECS limits
// ============================================================

constexpr std::size_t maxComponents = 128;


// ============================================================
// Component storage
// ============================================================

struct ComponentList : std::array<Component*, maxComponents>
{
    ComponentList()
    {
        fill(nullptr);
    }
};

using ComponentBitset = std::bitset<maxComponents>;