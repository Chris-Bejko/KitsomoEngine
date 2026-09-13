#pragma once

#include "AssetReference.h"

class Prefab : public AssetReference
{
public:
    Prefab() = default;
    explicit Prefab(const std::string& path);
    ~Prefab() override = default;

    Prefab(const Prefab& other) = default;
    Prefab& operator=(const Prefab& other) = default;

    Prefab(Prefab&& other) noexcept = default;
    Prefab& operator=(Prefab&& other) noexcept = default;

    bool Load() override;
    void Unload() override;
    bool IsLoaded() const override;
};
