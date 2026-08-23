#pragma once

#include <string>
#include <vector>
#include <utility>


class SerializedComponent;

class SerializedEntity
{
private:
    std::string guid;
    std::string name;
    std::string parentGUID;

    std::vector<SerializedComponent> components;

public:
    SerializedEntity() = default;

    SerializedEntity(const std::string& name,const std::string& guid)
    : guid(guid),name(name)
    {
    }

    // Identity
    const std::string& GetGUID() const
    {
        return guid;
    }

    void SetGUID(const std::string& value)
    {
        guid = value;
    }

    const std::string& GetName() const
    {
        return name;
    }

    void SetName(const std::string& value)
    {
        name = value;
    }

    // Hierarchy
    const std::string& GetParentGUID() const
    {
        return parentGUID;
    }

    void SetParentGUID(const std::string& value)
    {
        parentGUID = value;
    }

    bool HasParent() const
    {
        return !parentGUID.empty();
    }

    // Components
    std::vector<SerializedComponent>& GetComponents()
    {
        return components;
    }

    const std::vector<SerializedComponent>& GetComponents() const
    {
        return components;
    }

    void AddComponent(const SerializedComponent& component)
    {
        components.push_back(component);
    }

    void AddComponent(SerializedComponent&& component)
    {
        components.push_back(std::move(component));
    }
};