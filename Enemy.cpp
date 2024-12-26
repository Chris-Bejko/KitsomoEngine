#include "Enemy.h"
#include "Entity.h"
#include "Components/Sprite.h"

void Enemy::update(float dt)
{
	if (target == nullptr)
		return;


}

void Enemy::Awake()
{

}

bool Enemy::Init()
{
	entity->AddComponent<Sprite>("triangle");
	return true;
}

void Enemy::SetFollowTarget(Transform* target)
{
	this->target = target;
}
