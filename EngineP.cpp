#include "../Sprite.h"
#include "Engine.h"
#include <iostream>
#include "../AssetManager.h"
#include "../EntityManager.h"
#include "../Entity.h"
#include "../RenderContainer.h"
#include "../Rigidbody.h"
#include "SystemManager.h"
#include "../InputSystem.h"
//#include "Collision.h"
#include <cassert>
#include "Engine.h"
#include "Player.h"
#include "GameManager.h"

Engine* Engine::s_instance = nullptr;

Engine::Engine()
{
	isRunning = false;
	isEngine = true;
}

Engine::~Engine()
{
}

void Engine::Quit()
{
	isRunning = false;
}

void Engine::Init()
{
	this->window = new sf::RenderWindow(sf::VideoMode(1280, 720), "SFML works!");
	this->window = window;
	auto inputSystem = new InputSystem();
	this->inputSystem = inputSystem;
	SystemsManager::get().AddSystem(inputSystem);
	manager = new EntityManager();
	Entity* newEntity = new Entity();
	Entity* floorSquare = new Entity();
	Entity* floorSquare1 = new Entity();
	Entity* floorSquare2 = new Entity();
	Entity* floorSquare3 = new Entity();
	newEntity->AddComponent<Player>(true, Vector2F(100, 100), "player");
	floorSquare->AddComponent<FloorSquare>().Config(Vector2F(250,100), sf::Color(0, 128, 0, 255));
	floorSquare->AddComponent<FloorSquare>().Config(Vector2F(250, 100), sf::Color::Green);
	floorSquare1->AddComponent<FloorSquare>().Config(Vector2F(550, 159), sf::Color::Red);
	floorSquare2->AddComponent<FloorSquare>().Config(Vector2F(1000, 500), sf::Color::Magenta);
	floorSquare3->AddComponent<FloorSquare>().Config(Vector2F(500, 500), sf::Color::Cyan);
	manager->addEntity(newEntity);
	manager->addEntity(floorSquare);
	manager->addEntity(floorSquare1);
	manager->addEntity(floorSquare2);
	manager->addEntity(floorSquare3);
	isRunning = true;

}

void Engine::Clean()
{
	AssetManager::get().clean();
	window->clear();
	window->close();
	isRunning = false;
}

void Engine::Render()
{
	window->clear();
	manager->draw();
	window->display();
}

void Engine::Update()
{
	sf::Clock deltaClock;
	manager->update(dt);
	SystemsManager::get().Update();
	manager->Collisions();
	dt = deltaClock.restart().asSeconds();
}

void Engine::Events()
{
	sf::Event event;
	window->pollEvent(event);
	if (event.type == sf::Event::Closed)
		Clean();
}

sf::RenderWindow& Engine::GetWindow()
{
	return *window;
}

void Engine::Spawn(Entity* entity)
{
	manager->addEntity(entity);
}