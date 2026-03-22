#include "PolygonCollider.h"
#include "../Engine.h"
#include "imgui.h"
#include "../Logger.h"
#include <cmath>
#include <limits>
#include "Transform.h"

PolygonCollider::PolygonCollider()
{
    collisionTag = "default";
    isTrigger = false;
}

PolygonCollider::PolygonCollider(std::string tag, std::vector<sf::Vector2f> vertices, bool isTrigger)
{
    collisionTag = tag;
    this->vertices = vertices;
    this->isTrigger = isTrigger;
}

bool PolygonCollider::Init()
{
    Collider::Init();
    LOG_DEBUG("PolygonCollider::OnInit called, vertices: ", vertices.size());
    if (vertices.empty())
    {
        vertices = {
            sf::Vector2f(0, -50),
            sf::Vector2f(50, 50),
            sf::Vector2f(-50, 50)};
    }
    RebuildVisual();
    SerializeVerticesToString();
    Field("vertices", verticesString);
    return true;
}

void PolygonCollider::SerializeVerticesToString()
{
    verticesString = "";
    for (size_t i = 0; i < vertices.size(); i++)
    {
        verticesString += std::to_string(vertices[i].x) + "|" +
                          std::to_string(vertices[i].y);
        if (i < vertices.size() - 1)
            verticesString += ";";
    }
    LOG_DEBUG("Serialized ", vertices.size(), " vertices: ", verticesString.c_str());
}

void PolygonCollider::DeserializeVerticesFromString(const std::string &str)
{
    vertices.clear();
    if (str.empty())
        return;

    std::stringstream ss(str);
    std::string pair;
    while (std::getline(ss, pair, ';'))
    {
        auto pipePos = pair.find('|');
        if (pipePos != std::string::npos)
        {
            try
            {
                float x = std::stof(pair.substr(0, pipePos));
                float y = std::stof(pair.substr(pipePos + 1));
                vertices.push_back(sf::Vector2f(x, y));
            }
            catch (...)
            {
                LOG_WARNING("Failed to parse vertex: ", pair.c_str());
            }
        }
    }
    LOG_DEBUG("Deserialized ", vertices.size(), " vertices");
    RebuildVisual();
}

void PolygonCollider::InitSerializedFields(ReadableSerializableVariableMap map)
{
    SerializableScript::InitSerializedFields(map);

    if (map.stringFields.count("vertices"))
        DeserializeVerticesFromString(map.stringFields["vertices"]);
}

std::vector<sf::Vector2f> PolygonCollider::GetWorldVertices()
{
    std::vector<sf::Vector2f> worldVerts;
    sf::Transform t;
    t.translate(entity->transform->position.x, entity->transform->position.y);
    t.rotate(entity->transform->rotation);
    t.scale(entity->transform->scale.x, entity->transform->scale.y);

    for (auto &v : vertices)
        worldVerts.push_back(t.transformPoint(v));

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

// SAT helpers
std::vector<sf::Vector2f> PolygonCollider::GetAxes(const std::vector<sf::Vector2f> &worldVerts)
{
    std::vector<sf::Vector2f> axes;
    for (size_t i = 0; i < worldVerts.size(); i++)
    {
        sf::Vector2f edge = worldVerts[(i + 1) % worldVerts.size()] - worldVerts[i];
        // Perpendicular (normal)
        sf::Vector2f normal(-edge.y, edge.x);
        // Normalize
        float len = std::sqrt(normal.x * normal.x + normal.y * normal.y);
        if (len > 0)
            normal /= len;
        axes.push_back(normal);
    }
    return axes;
}

void PolygonCollider::Project(const std::vector<sf::Vector2f> &verts, sf::Vector2f axis, float &min, float &max)
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

    // Test axes from both polygons
    for (auto &axes : {GetAxes(vertsA), GetAxes(vertsB)})
    {
        for (auto &axis : axes)
        {
            float minA, maxA, minB, maxB;
            Project(vertsA, axis, minA, maxA);
            Project(vertsB, axis, minB, maxB);
            if (maxA < minB || maxB < minA)
                return false; // Separating axis found
        }
    }
    return true;
}

bool PolygonCollider::SATvsBox(sf::FloatRect box)
{
    // Convert box to polygon vertices
    std::vector<sf::Vector2f> boxVerts = {
        {box.left, box.top},
        {box.left + box.width, box.top},
        {box.left + box.width, box.top + box.height},
        {box.left, box.top + box.height}};

    auto vertsA = GetWorldVertices();
    auto axesA = GetAxes(vertsA);
    auto axesB = GetAxes(boxVerts);

    for (auto &axes : {axesA, axesB})
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

bool PolygonCollider::SATvsCircle(sf::Vector2f center, float radius)
{
    auto worldVerts = GetWorldVertices();

    // Test polygon face normals
    for (size_t i = 0; i < worldVerts.size(); i++)
    {
        sf::Vector2f edge = worldVerts[(i + 1) % worldVerts.size()] - worldVerts[i];
        sf::Vector2f axis(-edge.y, edge.x);
        float len = std::sqrt(axis.x * axis.x + axis.y * axis.y);
        if (len > 0)
            axis /= len;

        float minA, maxA;
        Project(worldVerts, axis, minA, maxA);
        float circleProj = center.x * axis.x + center.y * axis.y;
        if (maxA < circleProj - radius || circleProj + radius < minA)
            return false;
    }

    // Test axis from closest vertex to circle center
    float minDist = std::numeric_limits<float>::max();
    sf::Vector2f closestVert;
    for (auto &v : worldVerts)
    {
        sf::Vector2f diff = center - v;
        float dist = diff.x * diff.x + diff.y * diff.y;
        if (dist < minDist)
        {
            minDist = dist;
            closestVert = v;
        }
    }

    sf::Vector2f axis = center - closestVert;
    float len = std::sqrt(axis.x * axis.x + axis.y * axis.y);
    if (len > 0)
        axis /= len;

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
        sf::Vector2f center(b.left + b.width / 2.f, b.top + b.height / 2.f);
        return SATvsCircle(center, b.width / 2.f);
    }
    return false;
}

void PolygonCollider::RebuildVisual()
{
    colliderVisual.setPointCount(vertices.size());
    for (size_t i = 0; i < vertices.size(); i++)
        colliderVisual.setPoint(i, vertices[i]);

    colliderVisual.setFillColor(sf::Color::Transparent);
    colliderVisual.setOutlineColor(sf::Color(255, 165, 0, 255)); // Orange
    colliderVisual.setOutlineThickness(1.f);
}

void PolygonCollider::DrawDebug()
{
    colliderVisual.setPosition(entity->transform->position.x, entity->transform->position.y);
    colliderVisual.setRotation(entity->transform->rotation);
    colliderVisual.setScale(entity->transform->scale.x, entity->transform->scale.y);
    Engine::get().GetWindow().draw(colliderVisual);

    // Draw vertex handles in edit mode
    if (editMode)
    {
        auto worldVerts = GetWorldVertices();
        for (size_t i = 0; i < worldVerts.size(); i++)
        {
            sf::CircleShape handle(6.f);
            handle.setOrigin(6.f, 6.f);
            handle.setPosition(worldVerts[i]);
            handle.setFillColor(i == (size_t)selectedVertex ? sf::Color::Yellow : sf::Color::White);
            handle.setOutlineColor(sf::Color(255, 165, 0));
            handle.setOutlineThickness(1.f);
            Engine::get().GetWindow().draw(handle);
        }

        if (addingVertex)
        {
            auto mousePixel = sf::Mouse::getPosition(Engine::get().GetWindow());
            auto mouseWorld = Engine::get().GetWindow().mapPixelToCoords(mousePixel);
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
    auto worldVerts = GetWorldVertices();

    float scaleX = entity->transform->scale.x;
    float scaleY = entity->transform->scale.y;

    // Find hovered vertex
    hoveredVertex = -1;
    for (size_t i = 0; i < worldVerts.size(); i++)
    {
        sf::Vector2f diff = mouseWorld - worldVerts[i];
        if (std::sqrt(diff.x * diff.x + diff.y * diff.y) < 10.f)
        {
            hoveredVertex = (int)i;
            break;
        }
    }

    if (sf::Mouse::isButtonPressed(sf::Mouse::Right))
    {
        if (addingVertex)
        {
            // Convert mouse world pos to local space
            sf::Transform inv;
            inv.translate(entity->transform->position.x, entity->transform->position.y);
            inv.rotate(entity->transform->rotation);
            inv.scale(scaleX, scaleY);
            sf::Vector2f localPos = inv.getInverse().transformPoint(mouseWorld);
            vertices.push_back(localPos);
            RebuildVisual();
            SerializeVerticesToString();
            addingVertex = false;
        }
        else if (hoveredVertex >= 0)
        {
            selectedVertex = hoveredVertex;
        }
    }

    // Drag selected vertex
    if (sf::Mouse::isButtonPressed(sf::Mouse::Right) && selectedVertex >= 0 && selectedVertex < (int)vertices.size())
    {
        sf::Transform inv;
        inv.translate(entity->transform->position.x, entity->transform->position.y);
        inv.rotate(entity->transform->rotation);
        inv.scale(scaleX, scaleY);
        vertices[selectedVertex] = inv.getInverse().transformPoint(mouseWorld);
        RebuildVisual();
        SerializeVerticesToString();
    }

    if (!sf::Mouse::isButtonPressed(sf::Mouse::Right))
        selectedVertex = -1;

    // Delete vertex with ctrl + Right click
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) && sf::Mouse::isButtonPressed(sf::Mouse::Right) && hoveredVertex >= 0 && vertices.size() > 3)
    {
        vertices.erase(vertices.begin() + hoveredVertex);
        hoveredVertex = -1;
        selectedVertex = -1;
        RebuildVisual();
        SerializeVerticesToString();
    }
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
        ImGui::PopStyleColor();

        if (vertices.size() > 3)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                               "Ctrl + Right-click vertex to delete");
            ImGui::PopStyleColor();
        }
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
