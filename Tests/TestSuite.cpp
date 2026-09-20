#include "ComponentTypeRegistry.h"
#include "CommandHistory.h"
#include "EntityClipboard.h"
#include "EventSystem.h"
#include "GUIDGenerator.h"
#include "Logger.h"
#include "Prefab.h"
#include "SerializedComponent.h"
#include "SerializedEntity.h"
#include "SystemManager.h"
#include "Vector2.h"

#include <cmath>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <regex>
#include <string>
#include <typeindex>
#include <unordered_set>

namespace
{
    class TestComponentA {};
    class TestComponentB {};

    struct ScoreEvent
    {
        int amount;
    };

    struct HealthEvent
    {
        int amount;
    };

    class CounterCommand final : public Command
    {
    public:
        CounterCommand(int& counter, int amount, std::string description)
            : counter(counter), amount(amount), description(std::move(description))
        {
        }

        void Execute() override
        {
            counter += amount;
        }

        void Undo() override
        {
            counter -= amount;
        }

        std::string GetDescription() const override
        {
            return description;
        }

    private:
        int& counter;
        int amount;
        std::string description;
    };

    class RecordingSystem final : public System
    {
    public:
        explicit RecordingSystem(int& updateCount)
            : updateCount(updateCount)
        {
        }

        void Update() override
        {
            ++updateCount;
        }

    private:
        int& updateCount;
    };

    class TestAssetReference final : public AssetReference
    {
    public:
        using AssetReference::AssetReference;

        bool Load() override
        {
            ++loadCount;
            loaded = HasPath();
            return HasPath();
        }

        void Unload() override
        {
            ++unloadCount;
            loaded = false;
        }

        bool IsLoaded() const override
        {
            return loaded;
        }

        std::filesystem::path Resolve(const std::string& assetPath)
        {
            return ResolvePath(assetPath);
        }

        int loadCount = 0;
        int unloadCount = 0;

    private:
        bool loaded = false;
    };

    int failures = 0;

    void Check(bool condition, const char* expression, const char* testName)
    {
        if (condition)
            return;

        ++failures;
        std::cerr << "FAILED: " << testName << " - " << expression << '\n';
    }

#define CHECK(testName, ...) Check((__VA_ARGS__), #__VA_ARGS__, testName)

    bool NearlyEqual(float actual, float expected)
    {
        return std::fabs(actual - expected) < 0.0001f;
    }

    void RegistersAndFindsAName()
    {
        const char* testName = "RegistersAndFindsAName";
        auto& registry = ComponentTypeRegistry::get();
        const std::string name = "UnitTestComponent_RegistersAndFindsAName";

        const ComponentID createdID = registry.GetOrCreateID(name);

        CHECK(testName, createdID != INVALID_COMPONENT_ID);
        CHECK(testName, registry.GetID(name) == createdID);
        CHECK(testName, registry.GetOrCreateID(name) == createdID);
    }

    void UnknownNamesAreInvalid()
    {
        const char* testName = "UnknownNamesAreInvalid";
        auto& registry = ComponentTypeRegistry::get();

        CHECK(testName, registry.GetID("UnitTestComponent_DoesNotExist") == INVALID_COMPONENT_ID);
    }

    void AssociatesAndUnregistersTypes()
    {
        const char* testName = "AssociatesAndUnregistersTypes";
        auto& registry = ComponentTypeRegistry::get();
        const ComponentID componentID = registry.GetOrCreateID("UnitTestComponent_AssociatesAndUnregistersTypes");

        registry.AssociateType<TestComponentA>(componentID);

        CHECK(testName, registry.GetID<TestComponentA>() == componentID);
        CHECK(testName, getComponentTypeID<TestComponentA>() == componentID);

        registry.UnregisterProjectTypes({std::type_index(typeid(TestComponentA))});

        CHECK(testName, registry.GetID<TestComponentA>() == INVALID_COMPONENT_ID);
        CHECK(testName, registry.GetID<TestComponentB>() == INVALID_COMPONENT_ID);
    }

    void ComponentRegistryStopsAtItsCapacity()
    {
        const char* testName = "ComponentRegistryStopsAtItsCapacity";
        auto& registry = ComponentTypeRegistry::get();
        std::size_t registeredCount = 0;

        while (registry.GetOrCreateID("UnitTestCapacity_" + std::to_string(registeredCount)) != INVALID_COMPONENT_ID)
        {
            ++registeredCount;
        }

        CHECK(testName, registeredCount <= maxComponents);
        CHECK(testName, registry.GetOrCreateID("UnitTestCapacity_Overflow") == INVALID_COMPONENT_ID);
    }

    void Vector2ArithmeticAndComparison()
    {
        const char* testName = "Vector2ArithmeticAndComparison";
        const Vector2I left(3, -2);
        const Vector2I right(4, 5);

        CHECK(testName, left + right == Vector2I(7, 3));
        CHECK(testName, left - right == Vector2I(-1, -7));
        CHECK(testName, left * 3 == Vector2I(9, -6));
        CHECK(testName, left / 2 == Vector2I(1, -1));
        CHECK(testName, left / 0 == Vector2I());
        CHECK(testName, left != right);

        Vector2I mutableVector(1, 2);
        mutableVector += Vector2I(3, 4);
        mutableVector -= Vector2I(1, 5);
        mutableVector *= 2;
        CHECK(testName, mutableVector == Vector2I(6, 2));
    }

    void VectorMathProducesExpectedResults()
    {
        const char* testName = "VectorMathProducesExpectedResults";
        const Vector2F vector(3.0f, 4.0f);
        const Vector2F other(6.0f, 8.0f);
        const Vector2F normalized = vector.normalized();

        CHECK(testName, NearlyEqual(vector.magnitudeSq(), 25.0f));
        CHECK(testName, NearlyEqual(vector.magnitude(), 5.0f));
        CHECK(testName, NearlyEqual(vector.dot(other), 50.0f));
        CHECK(testName, NearlyEqual(vector.distance(other), 5.0f));
        CHECK(testName, NearlyEqual(normalized.x, 0.6f));
        CHECK(testName, NearlyEqual(normalized.y, 0.8f));
        CHECK(testName, Vector2F().normalized() == Vector2F());
        CHECK(testName, vector.lerp(other, 0.5f) == Vector2F(4.5f, 6.0f));
        CHECK(testName, vector.toString() == "(3.000000, 4.000000)");
    }

    void Vector3AndVector4SupportCoreOperations()
    {
        const char* testName = "Vector3AndVector4SupportCoreOperations";
        const Vector3I xAxis(1, 0, 0);
        const Vector3I yAxis(0, 1, 0);
        const Vector4I vector(1, 2, 3, 4);

        CHECK(testName, xAxis.cross(yAxis) == Vector3I(0, 0, 1));
        CHECK(testName, Vector3I(1, 2, 3) + Vector3I(4, 5, 6) == Vector3I(5, 7, 9));
        CHECK(testName, vector * 2 == Vector4I(2, 4, 6, 8));
        CHECK(testName, vector / 0 == Vector4I());
    }

    void GuidGenerationUsesRfc4122VersionFourFormat()
    {
        const char* testName = "GuidGenerationUsesRfc4122VersionFourFormat";
        const std::regex guidPattern("^[0-9a-f]{8}-[0-9a-f]{4}-4[0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12}$");
        std::unordered_set<std::string> generatedGUIDs;

        for (int index = 0; index < 100; ++index)
        {
            const std::string guid = EngineGUID::Generate();
            CHECK(testName, std::regex_match(guid, guidPattern));
            generatedGUIDs.insert(guid);
        }

        CHECK(testName, generatedGUIDs.size() == 100);
    }

    void CommandHistoryExecutesUndoesAndRedoes()
    {
        const char* testName = "CommandHistoryExecutesUndoesAndRedoes";
        auto& history = CommandHistory::get();
        history.Clear();
        int counter = 0;

        history.Execute(std::make_unique<CounterCommand>(counter, 4, "Add four"));
        CHECK(testName, counter == 4);
        CHECK(testName, history.CanUndo());
        CHECK(testName, !history.CanRedo());
        CHECK(testName, history.GetUndoDescription() == "Add four");

        history.Undo();
        CHECK(testName, counter == 0);
        CHECK(testName, !history.CanUndo());
        CHECK(testName, history.CanRedo());
        CHECK(testName, history.GetRedoDescription() == "Add four");

        history.Redo();
        CHECK(testName, counter == 4);
        CHECK(testName, history.CanUndo());
        CHECK(testName, !history.CanRedo());

        history.Undo();
        history.Execute(std::make_unique<CounterCommand>(counter, 2, "Add two"));
        CHECK(testName, counter == 2);
        CHECK(testName, !history.CanRedo());
        CHECK(testName, history.GetUndoDescription() == "Add two");

        history.Clear();
        CHECK(testName, !history.CanUndo());
        CHECK(testName, !history.CanRedo());
        CHECK(testName, history.GetUndoDescription().empty());
        CHECK(testName, history.GetRedoDescription().empty());
    }

    void EventSystemDispatchesAndUnsubscribesSafely()
    {
        const char* testName = "EventSystemDispatchesAndUnsubscribesSafely";
        auto& events = EventSystem::get();
        events.Clear();
        int firstTotal = 0;
        int secondTotal = 0;
        int lateTotal = 0;
        EventSystem::CallbackId secondListener = 0;

        events.Subscribe<ScoreEvent>([&](const ScoreEvent& event)
        {
            firstTotal += event.amount;
            events.Unsubscribe<ScoreEvent>(secondListener);
            events.Subscribe<ScoreEvent>([&](const ScoreEvent& nextEvent)
            {
                lateTotal += nextEvent.amount;
            });
        });
        secondListener = events.Subscribe<ScoreEvent>([&](const ScoreEvent& event)
        {
            secondTotal += event.amount;
        });

        events.Fire(ScoreEvent{3});
        CHECK(testName, firstTotal == 3);
        CHECK(testName, secondTotal == 0);
        CHECK(testName, lateTotal == 0);

        events.Fire(ScoreEvent{2});
        CHECK(testName, firstTotal == 5);
        CHECK(testName, secondTotal == 0);
        CHECK(testName, lateTotal == 2);
        events.Clear();
    }

    void EventSystemKeepsEventChannelsIndependent()
    {
        const char* testName = "EventSystemKeepsEventChannelsIndependent";
        auto& events = EventSystem::get();
        events.Clear();
        int scoreTotal = 0;
        int healthTotal = 0;

        events.Subscribe<ScoreEvent>([&](const ScoreEvent& event)
        {
            scoreTotal += event.amount;
        });
        events.Subscribe<HealthEvent>([&](const HealthEvent& event)
        {
            healthTotal += event.amount;
        });

        events.Fire(ScoreEvent{7});
        CHECK(testName, scoreTotal == 7);
        CHECK(testName, healthTotal == 0);

        events.Fire(HealthEvent{4});
        CHECK(testName, scoreTotal == 7);
        CHECK(testName, healthTotal == 4);
        events.Clear();
    }

    void SerializationPreservesEntityAndComponentData()
    {
        const char* testName = "SerializationPreservesEntityAndComponentData";
        SerializedComponent sprite("Sprite", "component-guid");
        sprite.AddSerializedField("texture", "Assets/Textures/player.png");
        sprite.AddSerializedField("layer", "3");
        sprite.AddSerializedField("layer", "4");

        CHECK(testName, sprite.GetType() == "Sprite");
        CHECK(testName, sprite.GetGUID() == "component-guid");
        CHECK(testName, sprite.GetFields().size() == 2);
        CHECK(testName, sprite.GetFields().at("layer") == "4");

        SerializedEntity entity("Player", "entity-guid");
        entity.SetParentGUID("parent-guid");
        entity.AddComponent(sprite);
        entity.AddComponent(SerializedComponent("Rigidbody", "body-guid"));

        CHECK(testName, entity.GetName() == "Player");
        CHECK(testName, entity.GetGUID() == "entity-guid");
        CHECK(testName, entity.HasParent());
        CHECK(testName, entity.GetParentGUID() == "parent-guid");
        CHECK(testName, entity.GetComponents().size() == 2);
        CHECK(testName, entity.GetComponents().at(0).GetFields().at("texture") == "Assets/Textures/player.png");

        entity.SetParentGUID("");
        entity.SetName("RenamedPlayer");
        entity.SetGUID("new-entity-guid");
        CHECK(testName, !entity.HasParent());
        CHECK(testName, entity.GetName() == "RenamedPlayer");
        CHECK(testName, entity.GetGUID() == "new-entity-guid");
    }

    void LoggerDeliversLevelAndMessageToCallback()
    {
        const char* testName = "LoggerDeliversLevelAndMessageToCallback";
        auto& logger = Logger::get();
        int callbackCount = 0;
        LogLevel receivedLevel = LogLevel::Info;
        std::string receivedMessage;

        logger.SetCallback([&](LogLevel level, const std::string& message)
        {
            ++callbackCount;
            receivedLevel = level;
            receivedMessage = message;
        });
        logger.Log(LogLevel::Warning, "unit-test-message-", 42);
        logger.SetCallback({});

        CHECK(testName, callbackCount == 1);
        CHECK(testName, receivedLevel == LogLevel::Warning);
        CHECK(testName, receivedMessage.find("[WARNING]") != std::string::npos);
        CHECK(testName, receivedMessage.find("unit-test-message-42") != std::string::npos);
    }

    void ClipboardCopiesAndClearsSerializedEntities()
    {
        const char* testName = "ClipboardCopiesAndClearsSerializedEntities";
        auto& clipboard = EntityClipboard::get();
        clipboard.Clear();

        SerializedEntityClipboard source;
        source.name = "Copied Entity";
        SerializedComponent component("Sprite", "sprite-guid");
        component.AddSerializedField("texture", "Assets/Textures/player.png");
        source.components.push_back(component);

        clipboard.Copy(source);
        source.name = "Changed Source";
        source.components.front().AddSerializedField("texture", "Assets/Textures/changed.png");
        const SerializedEntityClipboard copied = clipboard.GetClipboard();

        CHECK(testName, clipboard.HasContent());
        CHECK(testName, copied.name == "Copied Entity");
        CHECK(testName, copied.components.size() == 1);
        CHECK(testName, copied.components.front().GetType() == "Sprite");
        CHECK(testName, copied.components.front().GetFields().at("texture") == "Assets/Textures/player.png");

        clipboard.Clear();
        const SerializedEntityClipboard cleared = clipboard.GetClipboard();
        CHECK(testName, !clipboard.HasContent());
        CHECK(testName, cleared.name.empty());
        CHECK(testName, cleared.components.empty());
    }

    void SystemsUpdateRegisteredSystems()
    {
        const char* testName = "SystemsUpdateRegisteredSystems";
        int firstUpdates = 0;
        int secondUpdates = 0;
        auto& systems = SystemsManager::get();

        systems.AddSystem(new RecordingSystem(firstUpdates));
        systems.AddSystem(new RecordingSystem(secondUpdates));
        systems.Update();
        systems.Update();

        CHECK(testName, firstUpdates == 2);
        CHECK(testName, secondUpdates == 2);
    }

    void AssetsMaintainReferenceLifecycleAndPaths()
    {
        const char* testName = "AssetsMaintainReferenceLifecycleAndPaths";
        TestAssetReference asset;

        CHECK(testName, !asset.HasPath());
        CHECK(testName, asset.GetName().empty());
        asset.SetPath("Assets/Textures/player.png");
        CHECK(testName, asset.HasPath());
        CHECK(testName, asset.GetPath() == "Assets/Textures/player.png");
        CHECK(testName, asset.GetName() == "player.png");
        CHECK(testName, asset.unloadCount == 1);
        CHECK(testName, asset.loadCount == 1);
        CHECK(testName, asset.IsLoaded());

        const auto expectedPath = std::filesystem::current_path() / "Assets/Textures/player.png";
        CHECK(testName, asset.Resolve("Assets/Textures/player.png") == expectedPath);
        CHECK(testName, asset.Resolve(expectedPath.string()) == expectedPath);

        asset.SetPath("");
        CHECK(testName, !asset.HasPath());
        CHECK(testName, asset.unloadCount == 2);
        CHECK(testName, asset.loadCount == 1);

        Prefab prefab("Assets/Prefabs/player.prefab");
        CHECK(testName, prefab.HasPath());
        CHECK(testName, prefab.GetName() == "player.prefab");
        CHECK(testName, prefab.IsLoaded());
    }

    struct TestCase
    {
        const char* name;
        void (*run)();
    };

    const TestCase testCases[] = {
        {"Registry.RegistersAndFindsAName", RegistersAndFindsAName},
        {"Registry.UnknownNamesAreInvalid", UnknownNamesAreInvalid},
        {"Registry.AssociatesAndUnregistersTypes", AssociatesAndUnregistersTypes},
        {"Vector.ArithmeticAndComparison", Vector2ArithmeticAndComparison},
        {"Vector.MathProducesExpectedResults", VectorMathProducesExpectedResults},
        {"Vector.Vector3AndVector4SupportCoreOperations", Vector3AndVector4SupportCoreOperations},
        {"Guid.GenerationUsesRfc4122VersionFourFormat", GuidGenerationUsesRfc4122VersionFourFormat},
        {"CommandHistory.ExecutesUndoesAndRedoes", CommandHistoryExecutesUndoesAndRedoes},
        {"Registry.StopsAtItsCapacity", ComponentRegistryStopsAtItsCapacity},
        {"Events.DispatchesAndUnsubscribesSafely", EventSystemDispatchesAndUnsubscribesSafely},
        {"Events.KeepsEventChannelsIndependent", EventSystemKeepsEventChannelsIndependent},
        {"Serialization.PreservesEntityAndComponentData", SerializationPreservesEntityAndComponentData},
        {"Logger.DeliversLevelAndMessageToCallback", LoggerDeliversLevelAndMessageToCallback},
        {"Clipboard.CopiesAndClearsSerializedEntities", ClipboardCopiesAndClearsSerializedEntities},
        {"Systems.UpdatesRegisteredSystems", SystemsUpdateRegisteredSystems},
        {"Assets.MaintainsReferenceLifecycleAndPaths", AssetsMaintainReferenceLifecycleAndPaths},
    };

    bool RunTest(const TestCase& testCase)
    {
        const int failuresBefore = failures;
        testCase.run();

        if (failures == failuresBefore)
        {
            std::cout << "PASSED: " << testCase.name << '\n';
            return true;
        }

        return false;
    }
}

int main(int argc, char* argv[])
{
    if (argc == 2)
    {
        for (const auto& testCase : testCases)
        {
            if (std::strcmp(argv[1], testCase.name) == 0)
            {
                return RunTest(testCase) ? 0 : 1;
            }
        }

        std::cerr << "Unknown test: " << argv[1] << '\n';
        return 2;
    }

    for (const auto& testCase : testCases)
    {
        RunTest(testCase);
    }

    if (failures == 0)
    {
        std::cout << "All unit tests passed.\n";
        return 0;
    }

    std::cerr << failures << " test assertion(s) failed.\n";
    return 1;
}