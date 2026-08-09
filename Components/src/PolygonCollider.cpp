#include "PolygonCollider.h"
#include "Engine.h"
#include "imgui.h"
#include "Logger.h"
#include <cmath>
#include <limits>
#include "Transform.h"
#include "ComponentRegistry.h"

DECLARE_COMPONENT_RULES(PolygonCollider, true)
REGISTER_SERIALIZABLE_COMPONENT(PolygonCollider)

PolygonCollider::PolygonCollider()
{
    collisionTag = "default";
    isTrigger = false;
}

PolygonCollider::PolygonCollider(std::string tag, std::vector<Vector2F> verts, bool isTrigger)
{
    collisionTag = tag;
    this->vertices = verts;
    this->isTrigger = isTrigger;
}

bool PolygonCollider::Init()
{
    Collider::Init();
    if (vertices.empty())
    {
        vertices = {
            Vector2F(0, -50),
            Vector2F(50, 50),
            Vector2F(-50, 50)};
    }
    RebuildVisual();
    Field("vertices", vertices);
    return true;
}

void PolygonCollider::InitSerializedFields(ReadableSerializableVariableMap map)
{
    SerializableScript::InitSerializedFields(map);
    RebuildVisual();
}

std::vector<Vector2F> PolygonCollider::GetWorldVertices()
{
    std::vector<Vector2F> worldVerts;
    Vector2F worldPos = entity->transform->GetWorldPosition();
    float worldRot = entity->transform->GetWorldRotation();
    Vector2F worldScale = entity->transform->GetWorldScale();

    float rad = worldRot * 3.14159f / 180.f;
    float cosR = cos(rad);
    float sinR = sin(rad);

    for (auto &v : vertices)
    {
        float scaledX = v.x * worldScale.x;
        float scaledY = v.y * worldScale.y;
        float rotatedX = scaledX * cosR - scaledY * sinR;
        float rotatedY = scaledX * sinR + scaledY * cosR;
        worldVerts.push_back(Vector2F(worldPos.x + rotatedX, worldPos.y + rotatedY));
    }
    return worldVerts;
}

sf::FloatRect PolygonCollider::GetBounds()
{
    auto worldVerts = GetWorldVertices();
    if (worldVerts.empty())
        return sf::FloatRect();

    float minX = worldVerts[0].x, maxX = worldVerts[0].x;
    float minY = worldVerts[0].y, maxY = worldVerts[0].y;
    for (auto &v : worldVerts)
    {
        minX = std::min(minX, v.x);
        maxX = std::max(maxX, v.x);
        minY = std::min(minY, v.y);
        maxY = std::max(maxY, v.y);
    }
    return sf::FloatRect(minX, minY, maxX - minX, maxY - minY);
}

std::vector<Vector2F> PolygonCollider::GetAxes(const std::vector<Vector2F> &worldVerts)
{
    std::vector<Vector2F> axes;
    for (size_t i = 0; i < worldVerts.size(); i++)
    {
        Vector2F edge = worldVerts[(i + 1) % worldVerts.size()] - worldVerts[i];
        Vector2F normal(-edge.y, edge.x);
        float len = std::sqrt(normal.x * normal.x + normal.y * normal.y);
        if (len > 0)
        {
            normal.x /= len;
            normal.y /= len;
        }
        axes.push_back(normal);
    }
    return axes;
}

void PolygonCollider::Project(const std::vector<Vector2F> &verts, Vector2F axis, float &min, float &max)
{
    min = max = (verts[0].x * axis.x + verts[0].y * axis.y);
    for (auto &v : verts)
    {
        float proj = v.x * axis.x + v.y * axis.y;
        min = std::min(min, proj);
        max = std::max(max, proj);
    }
}

bool PolygonCollider::SATvsPolygon(PolygonCollider &other)
{
    auto vertsA = GetWorldVertices();
    auto vertsB = other.GetWorldVertices();
    for (auto &axes : {GetAxes(vertsA), GetAxes(vertsB)})
    {
        for (auto &axis : axes)
        {
            float minA, maxA, minB, maxB;
            Project(vertsA, axis, minA, maxA);
            Project(vertsB, axis, minB, maxB);
            if (maxA < minB || maxB < minA)
                return false;
        }
    }
    return true;
}

bool PolygonCollider::SATvsBox(sf::FloatRect box)
{
    std::vector<Vector2F> boxVerts = {
        {box.left, box.top},
        {box.left + box.width, box.top},
        {box.left + box.width, box.top + box.height},
        {box.left, box.top + box.height}};
    auto vertsA = GetWorldVertices();
    for (auto &axes : {GetAxes(vertsA), GetAxes(boxVerts)})
    {
        for (auto &axis : axes)
        {
            float minA, maxA, minB, maxB;
            Project(vertsA, axis, minA, maxA);
            Project(boxVerts, axis, minB, maxB);
            if (maxA < minB || maxB < minA)
                return false;
        }
    }
    return true;
}

bool PolygonCollider::SATvsCircle(Vector2F center, float radius)
{
    auto worldVerts = GetWorldVertices();
    for (size_t i = 0; i < worldVerts.size(); i++)
    {
        Vector2F edge = worldVerts[(i + 1) % worldVerts.size()] - worldVerts[i];
        Vector2F axis(-edge.y, edge.x);
        float len = std::sqrt(axis.x * axis.x + axis.y * axis.y);
        if (len > 0)
        {
            axis.x /= len;
            axis.y /= len;
        }

        float minA, maxA;
        Project(worldVerts, axis, minA, maxA);
        float circleProj = center.x * axis.x + center.y * axis.y;
        if (maxA < circleProj - radius || circleProj + radius < minA)
            return false;
    }

    float minDist = std::numeric_limits<float>::max();
    Vector2F closestVert;
    for (auto &v : worldVerts)
    {
        Vector2F diff = center - v;
        float dist = diff.x * diff.x + diff.y * diff.y;
        if (dist < minDist)
        {
            minDist = dist;
            closestVert = v;
        }
    }

    Vector2F axis = center - closestVert;
    float len = std::sqrt(axis.x * axis.x + axis.y * axis.y);
    if (len > 0)
    {
        axis.x /= len;
        axis.y /= len;
    }

    float minA, maxA;
    Project(worldVerts, axis, minA, maxA);
    float circleProj = center.x * axis.x + center.y * axis.y;
    if (maxA < circleProj - radius || circleProj + radius < minA)
        return false;
    return true;
}

bool PolygonCollider::Intersects(Collider &other)
{
    if (other.GetType() == ColliderType::Polygon)
        return SATvsPolygon(static_cast<PolygonCollider &>(other));
    if (other.GetType() == ColliderType::Box)
        return SATvsBox(other.GetBounds());
    if (other.GetType() == ColliderType::Circle)
    {
        sf::FloatRect b = other.GetBounds();
        return SATvsCircle(Vector2F(b.left + b.width / 2.f, b.top + b.height / 2.f), b.width / 2.f);
    }
    return false;
}

void PolygonCollider::RebuildVisual()
{
    colliderVisual.setPointCount(vertices.size());
    for (size_t i = 0; i < vertices.size(); i++)
        colliderVisual.setPoint(i, sf::Vector2f(vertices[i].x, vertices[i].y));
    colliderVisual.setFillColor(sf::Color::Transparent);
    colliderVisual.setOutlineColor(sf::Color(255, 165, 0, 255));
    colliderVisual.setOutlineThickness(1.f);
    Serialize();
}

void PolygonCollider::DrawDebug()
{
    Vector2F worldPos = entity->transform->GetWorldPosition();
    colliderVisual.setPosition(worldPos.x, worldPos.y);
    colliderVisual.setRotation(entity->transform->GetWorldRotation());
    Vector2F worldScale = entity->transform->GetWorldScale();
    colliderVisual.setScale(worldScale.x, worldScale.y);
    Engine::get().GetWindow().draw(colliderVisual);

    if (editMode)
    {
        auto worldVerts = GetWorldVertices();
        for (size_t i = 0; i < worldVerts.size(); i++)
        {
            sf::CircleShape handle(6.f);
            handle.setOrigin(6.f, 6.f);
            handle.setPosition(worldVerts[i].x, worldVerts[i].y);
            handle.setFillColor(i == (size_t)selectedVertex ? sf::Color::Yellow : sf::Color::White);
            handle.setOutlineColor(sf::Color(255, 165, 0));
            handle.setOutlineThickness(1.f);
            Engine::get().GetWindow().draw(handle);
        }

        if (addingVertex)
        {
            auto mouseWorld = Engine::get().GetWindow().mapPixelToCoords(
                sf::Mouse::getPosition(Engine::get().GetWindow()));
            sf::CircleShape preview(5.f);
            preview.setOrigin(5.f, 5.f);
            preview.setPosition(mouseWorld);
            preview.setFillColor(sf::Color(255, 165, 0, 150));
            Engine::get().GetWindow().draw(preview);
        }
    }
}

void PolygonCollider::draw()
{
    if (!Engine::get().isEngine)
        return;
    if (!editMode)
        return;
    DrawDebug();
}

void PolygonCollider::update(float dt)
{
    if (editMode)
    {
        RebuildVisual();
        UpdateEditMode();
    }
}

void PolygonCollider::updateEngine(float dt)
{
    if (editMode)
    {
        RebuildVisual();
        UpdateEditMode();
    }
}

void PolygonCollider::UpdateEditMode()
{
    if (ImGui::GetIO().WantCaptureMouse)
        return;

    auto mousePixel = sf::Mouse::getPosition(Engine::get().GetWindow());
    auto mouseWorld = Engine::get().GetWindow().mapPixelToCoords(mousePixel);
    Vector2F mouse(mouseWorld.x, mouseWorld.y);

    auto worldVerts = GetWorldVertices();
    Vector2F worldPos = entity->transform->GetWorldPosition();
    float worldRot = entity->transform->GetWorldRotation();
    Vector2F worldScale = entity->transform->GetWorldScale();

    // Convert world pos to local space
    auto toLocal = [&](Vector2F worldPoint) -> Vector2F
    {
        float rad = worldRot * 3.14159f / 180.f;
        float cosR = cos(-rad);
        float sinR = sin(-rad);
        float dx = worldPoint.x - worldPos.x;
        float dy = worldPoint.y - worldPos.y;
        return Vector2F(
            (dx * cosR - dy * sinR) / worldScale.x,
            (dx * sinR + dy * cosR) / worldScale.y);
    };

    // Find hovered vertex
    hoveredVertex = -1;
    for (size_t i = 0; i < worldVerts.size(); i++)
    {
        Vector2F diff = mouse - worldVerts[i];
        if (diff.magnitude() < 10.f)
        {
            hoveredVertex = (int)i;
            break;
        }
    }

    bool rightDown = sf::Mouse::isButtonPressed(sf::Mouse::Right);

    // Add vertex mode - left click to place
    if (addingVertex)
    {
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && !wasLeftDown)
        {
            vertices.push_back(toLocal(mouse));
            RebuildVisual();
            addingVertex = false;
        }
        wasLeftDown = sf::Mouse::isButtonPressed(sf::Mouse::Left);
        return; // don't do drag while adding
    }

    // Start drag on right click press (not hold)
    if (rightDown && !wasRightDown)
    {
        // Delete with ctrl
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) &&
            hoveredVertex >= 0 && vertices.size() > 3)
        {
            vertices.erase(vertices.begin() + hoveredVertex);
            hoveredVertex = -1;
            selectedVertex = -1;
            RebuildVisual();
            wasRightDown = true;
            return;
        }

        if (hoveredVertex >= 0)
            selectedVertex = hoveredVertex;
    }

    // Drag selected vertex
    if (rightDown && selectedVertex >= 0 && selectedVertex < (int)vertices.size())
    {
        Vector2F before = vertices[selectedVertex];
        vertices[selectedVertex] = toLocal(mouse);
        LOG_DEBUG("Vertex ", selectedVertex, " moved from ",
                  before.x, ",", before.y, " to ",
                  vertices[selectedVertex].x, ",", vertices[selectedVertex].y);
        RebuildVisual();
    }

    // Release
    if (!rightDown && wasRightDown)
    {
        selectedVertex = -1; // clear AFTER final position is set
    }

    wasRightDown = rightDown;
}

void PolygonCollider::DrawEditorButton()
{
    if (editMode)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.5f, 0.1f, 1.0f));
        if (ImGui::Button("Stop Editing", ImVec2(-1, 24)))
        {
            editMode = false;
            addingVertex = false;
        }
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.3f, 0.1f, 1.0f));
        if (ImGui::Button("+ Add Vertex", ImVec2(-1, 24)))
            addingVertex = !addingVertex;

        if (addingVertex)
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f),
                               "Left-click in viewport to place vertex");
        ImGui::PopStyleColor();

        if (vertices.size() > 3)
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                               "Ctrl + Right-click vertex to delete");
        ImGui::Text("Vertices: %zu", vertices.size());
    }
    else
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.4f, 0.7f, 1.0f));
        if (ImGui::Button("Edit Polygon", ImVec2(-1, 24)))
            editMode = true;
        ImGui::PopStyleColor();
    }
}