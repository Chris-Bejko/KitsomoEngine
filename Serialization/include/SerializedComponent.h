#pragma once
#include <map>
#include <string>
class SerializedComponent
{
public:
    SerializedComponent() = default;

    SerializedComponent(
        const std::string& componentName,
        const std::string& guid)
        : componentName(componentName), guid(guid)
    {
    }

    const std::string& GetType() const
    {
        return componentName;
    }

    const std::string& GetGUID() const
    {
        return guid;
    }

    std::map<std::string, std::string>& GetFields()
    {
        return fields;
    }

    const std::map<std::string, std::string>& GetFields() const
    {
        return fields;
    }

    void SetType(const std::string& name)
    {
        componentName = name;
    }

    void SetGUID(const std::string& value)
    {
        guid = value;
    }

    void AddSerializedField(const std::string& name, const std::string& value)
    {
        fields[name] = value;
    }

private:
    std::string componentName;
    std::string guid;
    std::map<std::string, std::string> fields;
};