#include "Player.h"
#include "Sprite.h"
#include "../Engine.h"
#include "FloorSquare.h"
#include "Bullet.h"
#include "../Time.h"

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
	std::cout << "Should add sprite" << std::endl;
	Serialize();
	std::cout << "Init called" << std::endl;
	AssetManager::get().loadTexture("triangle", "triangle.png");
	if (!entity->HasComponent<Sprite>())
	{
		std::cout << "ADDING SPRITE WITH RENDER ORDER 2" << std::endl;
		entity->AddComponent<Sprite>("triangle", 2);
	}
	entity->transform->scale = Vector2F(0.05, 0.05);
	return true;
}

void Player::Serialize()
{
	variables.push_back({ "lastColor", &lastColorString, char_Type });
}

std::vector<SerializableVariable>* Player::GetSerializedFields()
{
	return &variables;
}

void Player::InitSerializedFields(ReadableSerializableVariableMap map)
{
	for (auto const& [key, value] : map.stringFields)
	{
		if (key == "lastColor")
		{
			lastColor.SetColor(value);
			lastColorString = value;
		}
	}
}

void Player::Awake()
{
	//entity->transform->position = initPos;
	Entity* camera = new Entity("Camera");
	this->camera = &camera->AddComponent<Camera>();
	Engine::get().Spawn(camera);
	std::cout << "Awake called" << std::endl;
	entity->AddComponent<BoxCollider>(initTag, sf::FloatRect(0, 0, 55, 50));
	Entity* bulletSpawnPoint = new Entity("spawnpoint");
	AssetManager::get().loadTexture("circle", "circle.png");
	bulletSpawnPoint->AddComponent<Sprite>("circle");
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

	//std::cout << dt << std::endl;
	camera->Follow(sf::Vector2f(entity->GetComponent<Transform>().position.x, entity->GetComponent<Transform>().position.y));
	lastColor.SetColor(lastColorString);
	SetSpawnPointPosition();
	spawnPoint->rotation = entity->transform->rotation;
	timer += dt;
	//std::cout << "" << std::endl;
	std::cout << Time::deltaTime << std::endl;
	if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && timer >= cooldown)
	{
		timer = 0;

		Entity* bullet = new Entity("Bullet");
		std::cout << "Space pressed" << std::endl;

		auto spawned = &bullet->AddComponent<Bullet>();
		spawned->SetPosition(Vector2F(entity->transform->position.x, spawnPoint->position.y));
		//spawned->SetRotation(entity->transform->rotation);
		spawned->SetColor(lastColor);
		Engine::get().GetManager()->addEntity(bullet);
		spawned->AddForce(GetMouseVector());
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
	{
		Move(Vector2F(moveSpeed, 0));
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
	{
		Move(Vector2F(-moveSpeed, 0));
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
	{
		Move(Vector2F(0, -moveSpeed));
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
	{
		Move(Vector2F(0, moveSpeed));
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

void Player::OnCollisionEnter(BoxCollider& other)
{
	if (other.entity->HasComponent<FloorSquare>())
	{
		lastColor = other.entity->GetComponent<FloorSquare>().GetColor();
		entity->GetComponent<Sprite>().SetColor(lastColor.GetColor());
	}
}

void Player::OnCollisionExit(BoxCollider& other)
{
	entity->GetComponent<Sprite>().SetColor(sf::Color::Red);
}

Vector2F Player::GetMouseVector()
{
	std::cout << "GETTING MOUSE VETORRR" << std::endl;
	//sf::Vector2f currmouse(sf::Mouse::getPosition().x, sf::Mouse::getPosition().y);
	auto mousePos = sf::Mouse::getPosition(Engine::get().GetWindow());
	//auto mousePos = Engine::get().GetWindow().mapPixelToCoords(sf::Mouse::getPosition(Engine::get().GetWindow()));
	auto origin = sf::Vector2f(entity->GetComponent<Sprite>().GetOrigin().x, entity->GetComponent<Sprite>().GetOrigin().y);
	auto mousePosf = Vector2F(mousePos.x, mousePos.y);
	auto originF = Vector2F(origin.x, origin.y);
	auto aimDir = mousePosf - originF;
	auto aimDirNorm = atan2(aimDir.y, aimDir.x);
	auto finall = Vector2F(cos(aimDirNorm), sin(aimDirNorm));
	return finall;
}

void Player::Move(const Vector2F movement)
{
	entity->transform->Translate(movement);
}

void Player::LookAtMouse()
{
	sf::Vector2f currmouse(sf::Mouse::getPosition().x, sf::Mouse::getPosition().y);
	//mousePos = sf::Mouse::getPosition(Engine::get().GetWindow());
	mousePos = Engine::get().GetWindow().mapPixelToCoords(sf::Mouse::getPosition(Engine::get().GetWindow()));
	//sf::Vector2f testPos = Engine::get().GetWindow().mapPixelToCoords(sf::Mouse::getPosition());
	//std::cout << "test: " << testPos.x << "," << testPos.y << std::endl;
	//std::cout << "mouse: " << mousePos.x << "," << mousePos.y << std::endl;
	auto mouseAngle = -atan2(mousePos.x - entity->transform->position.x, mousePos.y - entity->transform->position.y) * 180 / 3.14159;

	entity->transform->LookAt(mouseAngle + 180);
}
