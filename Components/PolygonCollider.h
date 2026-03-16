#pragma once
#include "../Collision/Collider.h"
#include <vector>

class PolygonCollider : public Collider
{
public:
    PolygonCollider();
    PolygonCollider(std::string tag, std::vector<sf::Vector2f> vertices, bool isTrigger = false);

    bool Init() override;
    void updateEngine(float dt) override;
    void update(float dt) override;
    void draw() override;

    bool Intersects(Collider &other) override;
    sf::FloatRect GetBounds() override;
    void DrawDebug() override;
    ColliderType GetType() override { return ColliderType::Polygon; }

    std::vector<sf::Vector2f> GetWorldVertices();
    std::vector<sf::Vector2f> &GetLocalVertices() { return vertices; }

    void Serialize() override;
    void InitSerializedFields(ReadableSerializableVariableMap map) override final;
    void DrawEditorButton() override;

private:
    std::string verticesString; // For serialization
    std::vector<sf::Vector2f> vertices; // local space
    sf::ConvexShape colliderVisual;

    // Edit mode
    void UpdateEditMode();
    int selectedVertex = -1;
    int hoveredVertex = -1;
    bool addingVertex = false;

    // SAT helpers
    std::vector<sf::Vector2f> GetAxes(const std::vector<sf::Vector2f> &worldVerts);
    void Project(const std::vector<sf::Vector2f> &verts, sf::Vector2f axis, float &min, float &max);
    bool SATvsPolygon(PolygonCollider &other);
    bool SATvsBox(sf::FloatRect box);
    bool SATvsCircle(sf::Vector2f center, float radius);

    void RebuildVisual();
};