#pragma once
#include "Collider.h"
#include "Vector2.h"
#include <vector>

class PolygonCollider : public Collider
{
public:
    PolygonCollider();
    PolygonCollider(std::string tag, std::vector<Vector2F> vertices, bool isTrigger = false);

    bool Init() override;
    void updateEngine(float dt) override;
    void update(float dt) override;
    void draw() override;

    bool Intersects(Collider &other) override;
    sf::FloatRect GetBounds() override;
    void DrawDebug() override;
    ColliderType GetType() override { return ColliderType::Polygon; }

    std::vector<Vector2F> GetWorldVertices();
    std::vector<Vector2F> &GetLocalVertices() { return vertices; }

    void DrawEditorButton() override;
    void InitSerializedFields(ReadableSerializableVariableMap map) override;
    void OnFieldChanged(const std::string &fieldName) override
    {
        if (fieldName == "vertices")
            RebuildVisual();
    }

private:
    std::vector<Vector2F> vertices;
    sf::ConvexShape colliderVisual;

    void UpdateEditMode();
    int selectedVertex = -1;
    int hoveredVertex = -1;
    bool addingVertex = false;

    std::vector<Vector2F> GetAxes(const std::vector<Vector2F> &worldVerts);
    void Project(const std::vector<Vector2F> &verts, Vector2F axis, float &min, float &max);
    bool SATvsPolygon(PolygonCollider &other);
    bool SATvsBox(sf::FloatRect box);
    bool SATvsCircle(Vector2F center, float radius);
    void RebuildVisual();
    bool wasRightDown = false;
    bool wasLeftDown = false;
};