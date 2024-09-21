#include "Components/Sprite.h"
#include "Engine.h"
#include <iostream>
#include "../AssetManager.h"
#include "../EntityManager.h"
#include "../Entity.h"
#include "../RenderContainer.h"
#include "Components/Rigidbody.h"
#include "SystemManager.h"
#include "../InputSystem.h"
//#include "Collision.h"
#include <cassert>
#include "Engine.h"
#include "Components/Player.h"
#include "GameManager.h"
#include  "imgui.h"
#include  "imgui-sfml.h"
#include "imguiHandler.h"
#include <fstream>

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
	playMode = false;
	this->window = new sf::RenderWindow(sf::VideoMode(1280, 720), "SFML works!");
	this->window = window;
	if (!ImGui::SFML::Init(GetWindow()))
	{
		std::cerr << "Error initializing IMGUI window" << std::endl;
	}
	else
	{
		std::cout << "Window initialized" << std::endl;
	}
	auto inputSystem = new InputSystem();
	this->inputSystem = inputSystem;
	SystemsManager::get().AddSystem(inputSystem);
	manager = new EntityManager();
	std::ofstream txtFile;
	Load();
	//Entity* newEntity = new Entity("Player");
	//Entity* floorSquare = new Entity("floor Square");
	//Entity* floorSquare1 = new Entity("floor Square(1)");
	//Entity* floorSquare2 = new Entity("floor Square(2)");
	//Entity* floorSquare3 = new Entity("floorSquare(3)");
	//newEntity->AddComponent<Player>(true, Vector2F(100, 100), "player");
	//floorSquare->AddComponent<FloorSquare>().Config(Vector2F(250, 100), sf::Color(0, 128, 0, 255));
	//floorSquare1->AddComponent<FloorSquare>().Config(Vector2F(550, 159), sf::Color::Red);
	//floorSquare2->AddComponent<FloorSquare>().Config(Vector2F(1000, 500), sf::Color::Magenta);
	//floorSquare3->AddComponent<FloorSquare>().Config(Vector2F(500, 500), sf::Color::Cyan);
	//manager->addEntity(newEntity);
	//manager->addEntity(floorSquare);
	//manager->addEntity(floorSquare1);
	//manager->addEntity(floorSquare2);
	//manager->addEntity(floorSquare3);
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
	ImGui::SFML::Render(GetWindow());
	window->display();
}

void Engine::Update()
{
	bool checkBox;
	sf::Clock deltaClock;
	manager->update(dt);
	manager->Collisions();
	SystemsManager::get().Update();
	auto rest = deltaClock.restart();
	ImguiHandler::get().Update(rest);
	//ImGui::Begin("Entities");
	//ImGui::Button("+");
	//manager->DisplayEntities();
	//ImGui::End();



	dt = rest.asSeconds();
}

void Engine::Events()
{
	sf::Event event;
	window->pollEvent(event);
	ImGui::SFML::ProcessEvent(event);
	if (event.type == sf::Event::Closed)
		Clean();

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) && sf::Keyboard::isKeyPressed(sf::Keyboard::S))
		Save();
}

sf::RenderWindow& Engine::GetWindow()
{
	return *window;
}

void Engine::Spawn(Entity* entity)
{
	manager->addEntity(entity);
}


void Engine::Save()
{
	auto entitiesAndComps = manager->SerializeEntities();
	std::ofstream myFile;
	myFile.open("saveFile.txt");
	for (auto& e : entitiesAndComps)
	{
		std::cout << e.entityName << std::endl;
		myFile << "ENTITY_NAME_" << e.entityName << "_ENTITY_NAME_E_";
		for (auto& c : e.components)
		{
			std::cout << c.componentName << std::endl;
			myFile << "_COMPONENTS_LIST_";
			myFile << "_COMPONENT_NAME_" << c.componentName;
			myFile << "_COMPONENT_NAME_E_";
			for (auto& f : c.fields)
			{
				myFile << "_FIELD_";
				myFile << ":" << f.name << "::";
				myFile << "," << f.read() << ",,";
				myFile << ";" << f.type << ";;";
				std::cout << f.name << " , " << f.read() << std::endl;
				//myFile << "_FIELD_E_";
			}
			//myFile << "_COMPONENTS_LIST_";
		}
		myFile << "\n";
	}
	myFile.close();
	std::cout << "FIle saved?" << std::endl;
}

void Engine::Load()
{
	//convert file to std::vector<SerializableEntity>();
	std::vector<SerializableEntity> entities;
	std::vector<SerializableComponent> components;
	std::vector<SerializableVariable> fields;

	std::fstream myFile;
	std::string line;
	myFile.open("saveFile.txt", std::ios::out | std::ios::in);

	std::string entityName;
	std::string token;
	while (std::getline(myFile, line))
	{
		SerializableEntity ent;
		std::string delStart = "ENTITY_NAME_";
		std::string delEnd = "ENTITY_NAME_E_";

		auto first = line.find(delStart);
		auto last = line.find(delEnd);
		ent.entityName = line.substr(first + delStart.length(), last - delEnd.length() + 1);
		//std::cout << first << "," << last << std::endl;
		std::cout << ent.entityName << std::endl;
		line.erase(first, last - first + delEnd.length() + 1);
		std::size_t pos = 0;
		std::string delimiter = "_COMPONENTS_LIST_";
		//std::cout << line << std::endl;
		while ((pos = line.find(delimiter)) != std::string::npos)
		{
			delStart = "_COMPONENT_NAME_";
			delEnd = "_COMPONENT_NAME_E_";
			first = line.find(delStart);
			last = line.find(delEnd);
			SerializableComponent comp;
			std::cout << line.substr(first + delStart.length(), last - (first + delStart.length())) << std::endl;
			std::cout << first << "," << last << std::endl;
			comp.componentName = line.substr(first + delStart.length(), last - (first + delStart.length()));
			std::cout << comp.componentName << std::endl;
			line.erase(first, last - first + delEnd.length() + 1);

			std::size_t fieldPos = 0;
			std::string delField = "_FIELD_";
			while((fieldPos = line.find(delField)) != std::string::npos)
			{
				delStart = ":";
				delEnd = "::";
				first = line.find(delStart);
				last = line.find(delEnd);
				std::cout << line.substr(first + delStart.length(), last - (first + delStart.length())) << std::endl;
				//line.erase(first, last - first + delEnd.length() + 1);

				delStart = ",";
				delEnd = ",,";
				first = line.find(delStart);
				last = line.find(delEnd);
				std::cout << line.substr(first + delStart.length(), last - (first + delStart.length())) << std::endl;
				//line.erase(first, last - first + delEnd.length() + 1);

				delStart = ";";
				delEnd = ";;";
				first = line.find(delStart);
				last = line.find(delEnd);
				std::cout << line.substr(first + delStart.length(), last - (first + delStart.length())) << std::endl;
				//line.erase(first, last - first + delEnd.length() + 1);
				
				
				
				line.erase(0, fieldPos + delField.length());

			}
			//std::cout << line.substr(0, pos) << std::endl;
			std::cout << "\n";
			line.erase(0, pos + delimiter.length());
		}
		std::cout << line.length() << std::endl;
		//auto fieldsFirst = line.find_first_of("")
	}


	myFile.close();
	//std::vector<SerializableComponent> components;
	//std::vector<SerializableVariable> variables;
	//variables.push_back({ "position.x", 0, float_Type });
	//variables.push_back({ "position.y", 0, float_Type });
	//variables.push_back({ "rotation", 0, float_Type });
	//variables.push_back({ "scale.x" , 0, float_Type });
	//variables.push_back({ "scale.y", 0, float_Type });


	//components.push_back({ "Transform", variables });
	//entities.push_back({ "New Entity", components });
	for (auto& e : entities)
	{
		Entity* ent = new Entity(e.entityName);
		for (auto& c : e.components)
		{
			if (c.componentName == "Transform")
			{
				ent->AddComponent<Transform>().SetSerializedFields(c.fields);
			}
		}
		manager->addEntity(ent);
	}
}

