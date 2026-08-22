#pragma once
#include "base/Dialog.h"

class InspectorDialog : public Dialog
{
public:
    InspectorDialog() = default;
    virtual ~InspectorDialog() = default;

    void Draw() override;
    void Open() override;
    void Close() override;

private:
    void DrawEntity(Entity* entity);
    void DrawComponent(Component* component);
    void DrawComponents(Entity* entity);

    void DrawField(Entity* entity, Component* component, SerializableVariable& field, const std::string& fieldId);
    void DrawEntityRefField(std::string guidStorage, const std::string& fieldId);
    void DrawCompRefField(std::string packedStorage, const std::string& typeHint, const std::string& fieldId);
    void DrawVectorField(SerializableScript* script, const char* fieldName, const std::string& fieldId);
    void DrawVectorElement(const std::string& current, int fieldType, const std::string& elemId);

    void DrawAvailableComponents(Entity* entity);
    void HandleComponentDrop(Entity* entity);
    bool IsEntityValid(Entity* entity) const;
private:
    bool addingNewComp = false;
    std::vector<std::string> availableComponents;
};