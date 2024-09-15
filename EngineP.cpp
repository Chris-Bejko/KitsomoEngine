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

Engine* Engine::s_instance = nullptr;

Engine::Engine()
{
	isRunning = false;
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

	newEntity->AddComponent<Player>();
	manager->addEntity(newEntity);
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
	SystemsManager::get().Update();
	manager->update();
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

