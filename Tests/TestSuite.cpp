#include "ComponentTypeRegistry.h"
#include "CommandHistory.h"
#include "GUIDGenerator.h"
#include "Vector2.h"

#include <cmath>
#include <cstring>
#include <iostream>
#include <regex>
#include <string>
#include <typeindex>
#include <unordered_set>

namespace
{
    class TestComponentA {};
    class TestComponentB {};

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