#pragma once

#include "SerializedField.h"
#include "AssetReference.h"

class AssetRefField : public SerializedField
{
private:
    AssetReference *asset;

public:
    AssetRefField(const char *name, AssetReference *asset) : SerializedField(name), asset(asset)
    {
    }

    void Draw(const FieldDrawContext &context) override;

    std::string Serialize() override;

    void Deserialize(const std::string &serialized) override;

    void Resolve() override { };
};