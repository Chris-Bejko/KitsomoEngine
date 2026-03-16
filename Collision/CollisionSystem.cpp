#include "CollisionSystem.h"
#include <algorithm>
#include "../Engine.h"
#include "../Components/Rigidbody.h"
#include "../Components/Transform.h"
CollisionSystem *CollisionSystem::s_instance = nullptr;

bool CollisionSystem::AABB(sf::FloatRect a, sf::FloatRect b)
{
	return (a.left + a.width > b.left &&
			b.left + a.width > a.left &&
			b.top + b.height > a.top &&
			a.top + a.height > b.top);
}

void CollisionSystem::Update()
{
}
bool CollisionSystem::CheckCollision(Collider *a, Collider *b)
{
	if (!a || !b)
		return false;

	auto typeA = a->GetType();
	auto typeB = b->GetType();

	if (typeA == ColliderType::Box && typeB == ColliderType::Box)
		return AABB(a->GetBounds(), b->GetBounds());

	if (typeA == ColliderType::Circle && typeB == ColliderType::Circle)
		return a->Intersects(*b);

	if ((typeA == ColliderType::Box && typeB == ColliderType::Circle) ||
		(typeA == ColliderType::Circle && typeB == ColliderType::Box))
	{
		Collider *box = typeA == ColliderType::Box ? a : b;
		Collider *circle = typeA == ColliderType::Circle ? a : b;

		sf::FloatRect bounds = circle->GetBounds();
		sf::Vector2f center(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);
		float radius = bounds.width / 2.f;

		return AABBvsCircle(box->GetBounds(), center, radius);
	}

	 if (typeA == ColliderType::Polygon || typeB == ColliderType::Polygon)
    {
        Collider* poly = typeA == ColliderType::Polygon ? a : b;
        Collider* other = typeA == ColliderType::Polygon ? b : a;
        return poly->Intersects(*other);
    }


	return false;
}

sf::Vector2f CollisionSystem::GetCollisionNormal(Collider* a, Collider* b)
{
    sf::FloatRect boundsA = a->GetBounds();
    sf::FloatRect boundsB = b->GetBounds();

    sf::Vector2f centerA(boundsA.left + boundsA.width / 2.f,
                         boundsA.top  + boundsA.height / 2.f);
    sf::Vector2f centerB(boundsB.left + boundsB.width / 2.f,
                         boundsB.top  + boundsB.height / 2.f);

    sf::Vector2f diff = centerA - centerB;

    // Find smallest overlap axis
    float overlapX = (boundsA.width + boundsB.width)   / 2.f - std::abs(diff.x);
    float overlapY = (boundsA.height + boundsB.height) / 2.f - std::abs(diff.y);

    if (overlapX < overlapY)
        return sf::Vector2f(diff.x < 0 ? -1.f : 1.f, 0.f);
    else
        return sf::Vector2f(0.f, diff.y < 0 ? -1.f : 1.f);
}

float CollisionSystem::GetPenetrationDepth(Collider* a, Collider* b)
{
    sf::FloatRect boundsA = a->GetBounds();
    sf::FloatRect boundsB = b->GetBounds();

    sf::Vector2f centerA(boundsA.left + boundsA.width / 2.f,
                         boundsA.top  + boundsA.height / 2.f);
    sf::Vector2f centerB(boundsB.left + boundsB.width / 2.f,
                         boundsB.top  + boundsB.height / 2.f);

    sf::Vector2f diff = centerA - centerB;

    float overlapX = (boundsA.width + boundsB.width)   / 2.f - std::abs(diff.x);
    float overlapY = (boundsA.height + boundsB.height) / 2.f - std::abs(diff.y);

    return std::min(overlapX, overlapY);
}

void CollisionSystem::ResolveCollision(Collider* a, Collider* b)
{
    // Only resolve if at least one has a rigidbody
    bool aHasRB = a->entity->HasComponent<Rigidbody>();
    bool bHasRB = b->entity->HasComponent<Rigidbody>();
    if (!aHasRB && !bHasRB) return;

    Rigidbody* rbA = aHasRB ? &a->entity->GetComponent<Rigidbody>() : nullptr;
    Rigidbody* rbB = bHasRB ? &b->entity->GetComponent<Rigidbody>() : nullptr;

    // Skip if both static
    bool aStatic = !rbA || rbA->IsStatic();
    bool bStatic = !rbB || rbB->IsStatic();
    if (aStatic && bStatic) return;

    sf::Vector2f normal = GetCollisionNormal(a, b);
    float depth = GetPenetrationDepth(a, b);

    // Push-out resolution
    if (!aStatic && !bStatic)
    {
        float massA = rbA ? rbA->GetMass() : 1.f;
        float massB = rbB ? rbB->GetMass() : 1.f;
        float totalMass = massA + massB;
        a->entity->GetComponent<Transform>().Translate(
            Vector2F(normal.x * depth * (massB / totalMass),
                     normal.y * depth * (massB / totalMass)));
        b->entity->GetComponent<Transform>().Translate(
            Vector2F(-normal.x * depth * (massA / totalMass),
                     -normal.y * depth * (massA / totalMass)));
    }
    else if (!aStatic)
    {
        a->entity->GetComponent<Transform>().Translate(
            Vector2F(normal.x * depth, normal.y * depth));
    }
    else if (!bStatic)
    {
        b->entity->GetComponent<Transform>().Translate(
            Vector2F(-normal.x * depth, -normal.y * depth));
    }

    // Impulse resolution
    sf::Vector2f velA = rbA ? sf::Vector2f(rbA->GetVelocity().x, rbA->GetVelocity().y) : sf::Vector2f(0,0);
    sf::Vector2f velB = rbB ? sf::Vector2f(rbB->GetVelocity().x, rbB->GetVelocity().y) : sf::Vector2f(0,0);
    sf::Vector2f relVel = velA - velB;

    float velAlongNormal = relVel.x * normal.x + relVel.y * normal.y;
    if (velAlongNormal > 0) return; // already separating

    float bounciness = std::min(
        rbA ? rbA->GetBounciness() : 0.f,
        rbB ? rbB->GetBounciness() : 0.f);

    float massA = rbA ? rbA->GetMass() : std::numeric_limits<float>::infinity();
    float massB = rbB ? rbB->GetMass() : std::numeric_limits<float>::infinity();
    float invMassA = aStatic ? 0.f : 1.f / massA;
    float invMassB = bStatic ? 0.f : 1.f / massB;

    float j = -(1.f + bounciness) * velAlongNormal / (invMassA + invMassB);

    if (rbA && !aStatic)
        rbA->AddForce(Vector2F(j * invMassA * normal.x, j * invMassA * normal.y));
    if (rbB && !bStatic)
        rbB->AddForce(Vector2F(-j * invMassB * normal.x, -j * invMassB * normal.y));

    // Friction
    sf::Vector2f tangent(normal.y, -normal.x); // perpendicular to normal
    float velAlongTangent = relVel.x * tangent.x + relVel.y * tangent.y;
    float frictionCoeff = std::min(
        rbA ? rbA->GetFriction() : 0.f,
        rbB ? rbB->GetFriction() : 0.f);

    float jt = -velAlongTangent / (invMassA + invMassB);
    jt = std::clamp(jt, -std::abs(j) * frictionCoeff, std::abs(j) * frictionCoeff);

    if (rbA && !aStatic)
        rbA->AddForce(Vector2F(jt * invMassA * tangent.x, jt * invMassA * tangent.y));
    if (rbB && !bStatic)
        rbB->AddForce(Vector2F(-jt * invMassB * tangent.x, -jt * invMassB * tangent.y));
}

bool CollisionSystem::AABBvsCircle(sf::FloatRect box, sf::Vector2f circlePos, float radius)
{
	// Find closest point on box to circle center
	float closestX = std::clamp(circlePos.x, box.left, box.left + box.width);
	float closestY = std::clamp(circlePos.y, box.top, box.top + box.height);

	float dx = circlePos.x - closestX;
	float dy = circlePos.y - closestY;

	return (dx * dx + dy * dy) < (radius * radius);
}

bool CollisionSystem::ActiveCollision(Collider *a, Collider *b)
{
	return activeCollisions.count({a, b}) || activeCollisions.count({b, a});
}

void CollisionSystem::SetActive(Collider *a, Collider *b)
{
	activeCollisions.insert({a, b});
}

void CollisionSystem::SetInactive(Collider *a, Collider *b)
{
	activeCollisions.erase({a, b});
	activeCollisions.erase({b, a});
}