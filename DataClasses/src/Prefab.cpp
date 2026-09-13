#include "Prefab.h"

Prefab::Prefab(const std::string& path)
    : AssetReference(path)
{
}

bool Prefab::Load()
{
    return HasPath();
}

void Prefab::Unload()
{
}

bool Prefab::IsLoaded() const
{
    return HasPath();
}
