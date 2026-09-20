#include "Component.h"
#include "ComponentTypeRegistry.h"
#include "Entity.h"
#include "EntityManager.h"
#include "Serialization.h"
#include "SerializedComponent.h"
#include "Transform.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <iostream>
#include <string>

namespace
{
    int failures = 0;

    void Check(bool condition, const char* expression)
    {
        if (condition)
            return;

        ++failures;
        std::cerr << "FAILED: " << expression << '\n';
    }

#define CHECK(...) Check((__VA_ARGS__), #__VA_ARGS__)

    bool NearlyEqual(float actual, float expected)
    {
        return std::fabs(actual - expected) < 0.001f;
    }

    struct LifecycleCounters
    {
        int initialized = 0;
        int awakened = 0;
        int updated = 0;
        int engineUpdated = 0;
    };

    class LifecycleComponent final : public Component
    {
    public:
        explicit LifecycleComponent(LifecycleCounters& counters)
            : counters(counters)
        {
        }

        bool Init() override
        {
            ++counters.initialized;
            return true;
        }

        void Awake() override
        {
            ++counters.awakened;
        }

        void update(float) override
        {
            ++counters.updated;
        }

        void updateEngine(float) override
        {
            ++counters.engineUpdated;
        }

    private:
        LifecycleCounters& counters;
    };

    void EntityManagerLifecycle()
    {
        EntityManager manager;

        auto* firstEntity = new Entity("Enemy", "integration-enemy-1");
        auto* secondEntity = new Entity("Enemy", "integration-enemy-2");

        manager.addEntity(firstEntity);
        manager.addEntity(secondEntity);

        CHECK(manager.GetTotalEntities() == 0);
        CHECK(manager.GetUnvalidatedEntities().size() == 2);

        manager.ValidateAdded();

        CHECK(manager.GetTotalEntities() == 2);
        CHECK(manager.GetUnvalidatedEntities().empty());
        CHECK(firstEntity->GetName() == "Enemy");
        CHECK(secondEntity->GetName() == "Enemy (1)");
        CHECK(manager.GetUniqueName("Enemy") == "Enemy (2)");
        CHECK(manager.GetUniqueName("Enemy (1)") == "Enemy (2)");

        manager.ClearAllEntities();
        CHECK(manager.GetTotalEntities() == 0);
        CHECK(manager.GetUnvalidatedEntities().empty());
    }

    void EntityHierarchyAndTransforms()
    {
        Entity parent("Parent", "integration-parent");
        Entity child("Child", "integration-child");
        Entity secondParent("Second Parent", "integration-second-parent");

        parent.transform->position = Vector2F(10.0f, 20.0f);
        parent.transform->scale = Vector2F(2.0f, 3.0f);
        parent.transform->rotation = 90.0f;
        child.transform->position = Vector2F(4.0f, 0.0f);
        child.transform->scale = Vector2F(0.5f, 2.0f);
        child.transform->rotation = 15.0f;

        child.SetParent(&parent);
        const Vector2F worldPosition = child.transform->GetWorldPosition();
        const Vector2F worldScale = child.transform->GetWorldScale();

        CHECK(child.GetParent() == &parent);
        CHECK(parent.GetChildren().size() == 1);
        CHECK(parent.GetChildren().front() == &child);
        CHECK(child.transform->GetParent() == parent.transform);
        CHECK(NearlyEqual(worldPosition.x, 10.0f));
        CHECK(NearlyEqual(worldPosition.y, 28.0f));
        CHECK(worldScale == Vector2F(1.0f, 6.0f));
        CHECK(NearlyEqual(child.transform->GetWorldRotation(), 105.0f));

        child.SetParent(&secondParent);
        CHECK(child.GetParent() == &secondParent);
        CHECK(parent.GetChildren().empty());
        CHECK(secondParent.GetChildren().size() == 1);

        child.SetParent(nullptr);
        CHECK(!child.HasParent());
        CHECK(secondParent.GetChildren().empty());
        CHECK(child.transform->GetParent() == nullptr);
    }

    void EntityActiveHierarchy()
    {
        Entity parent("Parent", "active-parent");
        Entity child("Child", "active-child");
        child.SetParent(&parent);

        CHECK(parent.IsActiveInHierarchy());
        CHECK(child.IsActiveInHierarchy());

        parent.SetActive(false);
        CHECK(!parent.IsActiveInHierarchy());
        CHECK(!child.IsActiveInHierarchy());

        parent.SetActive(true);
        child.SetActive(false);
        CHECK(parent.IsActiveInHierarchy());
        CHECK(!child.IsActiveInHierarchy());

        child.SetActive(true);
        child.SetPendingDestroy(true);
        CHECK(!child.IsActiveInHierarchy());
    }

    void EntityManagerRemovesPendingEntities()
    {
        EntityManager manager;
        auto* firstEntity = new Entity("First", "remove-first");
        auto* secondEntity = new Entity("Second", "remove-second");
        manager.addEntity(firstEntity);
        manager.addEntity(secondEntity);
        manager.ValidateAdded();

        secondEntity->SetPendingDestroy(true);
        manager.ValidateRemoved();

        CHECK(manager.GetTotalEntities() == 1);
        CHECK(manager.GetEntities().front().get() == firstEntity);

        manager.RemoveEntityByGUID("remove-first");
        CHECK(manager.GetTotalEntities() == 0);
    }

    void EntityManagerTracksCollisionRecords()
    {
        EntityManager manager;
        std::vector<std::string> firstRecord = {"first", "second"};
        std::vector<std::string> secondRecord = {"third", "fourth"};

        manager.AddColliders(firstRecord);
        manager.AddColliders(secondRecord);
        CHECK(manager.GetActiveCollisions().size() == 2);

        manager.RemoveActiveCollision(firstRecord);
        CHECK(manager.GetActiveCollisions().size() == 1);
        CHECK(manager.GetActiveCollisions().front() == secondRecord);

        manager.RemoveActiveCollision(firstRecord);
        CHECK(manager.GetActiveCollisions().size() == 1);
    }

    void EntitySerializesComponentFields()
    {
        Entity entity("Serializable", "serialize-entity");
        entity.transform->position = Vector2F(12.5f, -4.0f);
        entity.transform->scale = Vector2F(2.0f, 3.0f);
        entity.transform->rotation = 45.0f;
        entity.ValidateAddedComponents();

        const auto components = entity.GetSerializedComponents();
        const auto transformIt = std::find_if(
            components.begin(),
            components.end(),
            [](const SerializedComponent& component)
            {
                return component.GetFields().count("position.x") > 0;
            });

        CHECK(transformIt != components.end());
        if (transformIt == components.end())
            return;

        const auto& fields = transformIt->GetFields();
        CHECK(fields.at("position.x") == "12.500000");
        CHECK(fields.at("position.y") == "-4.000000");
        CHECK(fields.at("rotation") == "45.000000");
        CHECK(fields.at("scale.x") == "2.000000");
        CHECK(fields.at("scale.y") == "3.000000");
    }

    void EntityComponentLifecycle()
    {
        const ComponentID componentID =
            ComponentTypeRegistry::get().RegisterType<LifecycleComponent>("IntegrationLifecycleComponent");
        CHECK(componentID != INVALID_COMPONENT_ID);

        LifecycleCounters counters;
        EntityManager manager;
        auto* entity = new Entity("Lifecycle", "lifecycle-entity");
        auto& component = entity->AddComponent<LifecycleComponent>(counters);

        CHECK(component.entity == entity);
        CHECK(counters.initialized == 1);
        CHECK(entity->HasComponent<LifecycleComponent>());
        CHECK(entity->GetUnvalidatedComponents().size() >= 3);

        manager.addEntity(entity);
        manager.Awake();
        CHECK(counters.awakened == 1);

        manager.update(0.016f);
        CHECK(counters.updated == 1);

        entity->SetActive(false);
        manager.update(0.016f);
        CHECK(counters.updated == 1);

        entity->RemoveComponent(&component);
        CHECK(!entity->HasComponent<LifecycleComponent>());
        CHECK(entity->GetComponentOfType<LifecycleComponent>() == nullptr);
    }

    void EntityManagerClearsInspectorState()
    {
        EntityManager manager;
        auto* firstEntity = new Entity("First", "inspector-first");
        auto* secondEntity = new Entity("Second", "inspector-second");
        manager.addEntity(firstEntity);
        manager.addEntity(secondEntity);
        manager.ValidateAdded();

        firstEntity->displayComponents = true;
        secondEntity->displayComponents = true;
        manager.ClearInspector();

        CHECK(!firstEntity->displayComponents);
        CHECK(!secondEntity->displayComponents);
    }

    void EntityManagerSerializesEntitySet()
    {
        EntityManager manager;
        auto* firstEntity = new Entity("Serialize First", "serialize-first");
        auto* secondEntity = new Entity("Serialize Second", "serialize-second");
        firstEntity->transform->position = Vector2F(8.0f, 9.0f);
        firstEntity->ValidateAddedComponents();
        secondEntity->ValidateAddedComponents();
        manager.addEntity(firstEntity);
        manager.addEntity(secondEntity);
        manager.ValidateAdded();

        const auto serialized = manager.SerializeEntities();

        CHECK(serialized.size() == 2);
    CHECK(serialized.at(0).entityName == "Serialize Second");
    CHECK(serialized.at(1).entityName == "Serialize First");
        CHECK(!serialized.at(0).components.empty());
        CHECK(!serialized.at(1).components.empty());
    }

    struct TestCase
    {
        const char* name;
        void (*run)();
    };

    const TestCase testCases[] = {
        {"EntityManagerLifecycle", EntityManagerLifecycle},
        {"EntityHierarchyAndTransforms", EntityHierarchyAndTransforms},
        {"EntityActiveHierarchy", EntityActiveHierarchy},
        {"EntityManagerRemovesPendingEntities", EntityManagerRemovesPendingEntities},
        {"EntityManagerTracksCollisionRecords", EntityManagerTracksCollisionRecords},
        {"EntitySerializesComponentFields", EntitySerializesComponentFields},
        {"EntityComponentLifecycle", EntityComponentLifecycle},
        {"EntityManagerClearsInspectorState", EntityManagerClearsInspectorState},
        {"EntityManagerSerializesEntitySet", EntityManagerSerializesEntitySet},
    };
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: ECSEngineIntegrationTests <test-name>\n";
        return 2;
    }

    for (const auto& testCase : testCases)
    {
        if (std::strcmp(argv[1], testCase.name) == 0)
        {
            testCase.run();
            if (failures == 0)
            {
                std::cout << "PASSED: Integration." << testCase.name << '\n';
                return 0;
            }
            return 1;
        }
    }

    std::cerr << "Unknown integration test: " << argv[1] << '\n';
    return 2;
}