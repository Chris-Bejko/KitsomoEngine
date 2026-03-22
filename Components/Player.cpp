#include "Player.h"
#include "Sprite.h"
#include "../Engine.h"
#include "FloorSquare.h"
#include "Bullet.h"
#include "AudioSource.h"
#include "../Timedelta.h"
#include "../Logger.h"

Player::Player()
{
	useControls = true;
	initTag = "Player";
}

Player::Player(bool useControls, Vector2F position, std::string tag)
{
	this->useControls = useControls;
	initPos = position;
	initTag = tag;
}

bool Player::Init()
{

	LOG_INFO("Player Inititalized");
	AssetManager::get().loadTexture("triangle", "triangle.png");
	if (!entity->HasComponent<Sprite>())
	{
		entity->AddComponent<Sprite>("triangle", 2, Color(ColorEnum::Red));
	}
	entity->transform->scale = Vector2F(0.05f, 0.05f);
	LOG_DEBUG("Player initialized with position: ", entity->transform->position.x, ", ", entity->transform->position.y, "and scale: ", entity->transform->scale.x, ", ", entity->transform->scale.y);
	Field("lastColorString", lastColorString);
	Field("bulletPrefab", bulletPrefab);
	Field("moveSpeed", moveSpeed);
	Field("cooldown", cooldown);
	Field("shootSound", shootSound);
	Field("floorTouchSound", floorTouchSound);
	return true;
}

void Player::Awake()
{
	// entity->transform->position = initPos;
	Entity *camera = new Entity("Camera");
	this->camera = &camera->AddComponent<Camera>();
	Engine::get().Spawn(camera);
	Entity *bulletSpawnPoint = new Entity("spawnpoint");
	AssetManager::get().loadTexture("circle", "circle.png");
	bulletSpawnPoint->AddComponent<Sprite>("circle");
	bulletSpawnPoint->ValidateAddedComponents();
	spawnPoint = bulletSpawnPoint->transform;
	spawnPoint->scale = Vector2F(.03f, .03f);
	Engine::get().Spawn(bulletSpawnPoint);
}

void Player::updateEngine(float dt)
{
	lastColor.SetColor(lastColorString);
}

void Player::update(float dt)
{

	camera->Follow(sf::Vector2f(entity->GetComponent<Transform>().position.x, entity->GetComponent<Transform>().position.y));
	// lastColor.SetColor(lastColorString);
	SetSpawnPointPosition();
	spawnPoint->rotation = entity->transform->rotation;
	timer += dt;
	// if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
	//{
	//	LOG_DEBUG("Mouse clicked! timer = ", timer, " cooldown = ", cooldown);
	//	if (timer >= cooldown)
	//		LOG_DEBUG("Spawning bullet!");
	//	else
	//		LOG_DEBUG("Timer not ready: ", timer, "/", cooldown);
	// }
	if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && timer >= cooldown)
	{
		timer = 0;

		Entity *bullet = Engine::get().SpawnPrefab(
			bulletPrefab,
			Vector2F(spawnPoint->position.x, spawnPoint->position.y));

		if (bullet && bullet->HasComponent<Bullet>())
		{
			auto &spawned = bullet->GetComponent<Bullet>();
			Vector2F mouseVec = GetMouseVector();
			// mouseVec.x += currentVelocity.x * dt;
			// mouseVec.y += currentVelocity.y * dt;
			spawned.AddForce(mouseVec);
			spawned.SetColor(lastColor);
		}

		if (entity->HasComponent<AudioSource>())
		{
			auto &audio = entity->GetComponent<AudioSource>();
			audio.LoadAudio(shootSound);
			audio.Play();
		}
		Engine::get().GetManager()->addEntity(bullet);
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
	{
		Move(Vector2F(moveSpeed * dt, 0));
		currentVelocity.x = moveSpeed;
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
	{
		Move(Vector2F(-moveSpeed * dt, 0));
		currentVelocity.x = -moveSpeed;
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
	{
		Move(Vector2F(0, -moveSpeed * dt));
		currentVelocity.y = -moveSpeed;
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
	{
		Move(Vector2F(0, moveSpeed * dt));
		currentVelocity.y = moveSpeed;
	}

	LookAtMouse();
}

void Player::SetSpawnPointPosition()
{
	auto bounds = entity->GetComponent<Sprite>().GetGlobalBounds();
	spawnPoint->entity->GetComponent<Sprite>().SetOrigin(Vector2F(bounds.left + bounds.width / 2, bounds.top));
	sf::Transform rot;
	rot.rotate(entity->transform->rotation, entity->transform->position.x, entity->transform->position.y);
	sf::Vector2f point = rot.transformPoint(entity->transform->position.x, entity->transform->position.y - 20);

	spawnPoint->position = Vector2F(point.x, point.y);
	spawnPoint->rotation = entity->transform->rotation;
}

void Player::OnTriggerEnter(Collider &other)
{
	LOG_DEBUG("Player triggered with something", other.GetCollisionTag().c_str());
	if (other.entity->HasComponent<FloorSquare>())
	{

		Color otherColor = other.entity->GetComponent<FloorSquare>().GetColorEnum();
		if (otherColor.SerializeColor() == lastColorString)
		{
			LOG_DEBUG("Player triggered with floor square of the same color, no color change");
			return;
		}
		if (entity->HasComponent<AudioSource>())
		{
			auto &audio = entity->GetComponent<AudioSource>();
			audio.LoadAudio(floorTouchSound);
			audio.Play();
		}
		lastColor = otherColor;
		lastColorString = lastColor.SerializeColor();

		LOG_DEBUG("Player triggered with FloorSquare, changing color to match the floor square's color: ", lastColor.SerializeColor());
		// entity->GetComponent<Sprite>().SetColor(lastColor);
	}
}

void Player::OnTriggerExit(Collider &other)
{
	// Color color(ColorEnum::Red);
	// entity->GetComponent<Sprite>().SetColor(color);
}

Vector2F Player::GetMouseVector()
{
	// sf::Vector2f currmouse(sf::Mouse::getPosition().x, sf::Mouse::getPosition().y);
	auto mousePos = Engine::get().GetWindow().mapPixelToCoords(sf::Mouse::getPosition(Engine::get().GetWindow()));
	// auto mousePos = sf::Mouse::getPosition(Engine::get().GetWindow());
	// auto mousePos = Engine::get().GetWindow().mapPixelToCoords(sf::Mouse::getPosition(Engine::get().GetWindow()));
	auto origin = sf::Vector2f(entity->transform->position.x, entity->transform->position.y);
	auto aimDirNorm = atan2(mousePos.y - origin.y, mousePos.x - origin.x);
	auto finall = Vector2F(cos(aimDirNorm) * 1.f, sin(aimDirNorm) * 1.0f);
	return finall;
}

void Player::Move(const Vector2F movement)
{
	entity->transform->Translate(movement);
}

void Player::LookAtMouse()
{
	sf::Vector2f currmouse(sf::Mouse::getPosition().x, sf::Mouse::getPosition().y);
	// mousePos = sf::Mouse::getPosition(Engine::get().GetWindow());
	mousePos = Engine::get().GetWindow().mapPixelToCoords(sf::Mouse::getPosition(Engine::get().GetWindow()));
	// sf::Vector2f testPos = Engine::get().GetWindow().mapPixelToCoords(sf::Mouse::getPosition());
	// std::cout << "test: " << testPos.x << "," << testPos.y << std::endl;
	// std::cout << "mouse: " << mousePos.x << "," << mousePos.y << std::endl;
	auto mouseAngle = -atan2(mousePos.x - entity->transform->position.x, mousePos.y - entity->transform->position.y) * 180 / 3.14159;

	entity->transform->LookAt(mouseAngle + 180);
}
