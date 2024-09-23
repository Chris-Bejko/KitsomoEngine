#include "Engine.h"
#include <iostream>
#include "SystemManager.h"
#include <cassert>
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

bool Engine::IsRunning()
{
	return isRunning;
}

sf::RenderWindow& Engine::GetWindow()
{
	return *window;
}

EntityManager* Engine::GetManager()
{
	return manager;
}

bool Engine::IsPlayMode()
{
	return playMode;
}

void Engine::Spawn(Entity* entity)
{
	manager->addEntity(entity);
}

size_t Engine::GetTotalEntities()
{
	return manager->GetTotalEntities();
}

void Engine::Save()
{
	auto entitiesAndComps = manager->SerializeEntities();
	std::ofstream myFile;
	myFile.open("saveFile.txt");
	for (auto& e : entitiesAndComps)
	{
		myFile << "ENTITY_NAME_" << e.entityName << "_ENTITY_NAME_E_";
		for (auto& c : e.components)
		{
			myFile << "_COMPONENTS_LIST_";
			myFile << "_COMPONENT_NAME_" << c.componentName;
			myFile << "_COMPONENT_NAME_E_";
			for (auto& f : c.variables)
			{
				myFile << "_FIELD_";
				myFile << ":" << f.name << "::";
				switch (f.type)
				{
				case 1:
				{
					myFile << "," << f.read() << ",,";
					break;
				}
				case 2:
				{
					myFile << "," << f.read() << ",,";
					break;
				}
				case 3:
				{
					std::string p = *reinterpret_cast<std::string*>(f.data);
					myFile << "," << p << ",,";
					break;
				}
				case 4:
				{
					bool p = *reinterpret_cast<bool*>(f.data);
					myFile << "," << p << ",,";
					break;
				}
				default:
					break;
				}
				myFile << ";" << f.type << ";;";
			}
			myFile << "_FIELD_";
		}
		myFile << "\n";
	}
	myFile.close();
}

void Engine::Load()
{
	//convert file to std::vector<SerializableEntity>();
	std::vector<SerializableEntity> entities;
	std::vector<SerializableComponent> components;

	std::fstream myFile;
	std::string line;
	myFile.open("saveFile.txt", std::ios::out | std::ios::in);

	std::string entityName;
	while (std::getline(myFile, line))
	{
		SerializableEntity ent;
		std::string delStart = "ENTITY_NAME_";
		std::string delEnd = "_ENTITY_NAME_E_";
		ent.entityName = GetSubstring(line, delStart, delEnd, true);
		std::size_t pos = 0;
		std::string delimiter = "_COMPONENTS_LIST_";
		while ((pos = line.find(delimiter)) != std::string::npos)
		{
			ReadableSerializableVariableMap fields;
			delStart = "_COMPONENT_NAME_";
			delEnd = "_COMPONENT_NAME_E_";
			SerializableComponent comp;
			comp.componentName = GetSubstring(line, delStart, delEnd, true);

			std::size_t fieldPos = 0;
			std::string delField = "_FIELD_";
			line.erase(0, pos + delimiter.length());
			while ((fieldPos = line.find(delField)) != std::string::npos)
			{
				delStart = ":";
				delEnd = "::";
				auto fieldName = GetSubstring(line, delStart, delEnd, false);

				delStart = ",";
				delEnd = ",,";
				auto fieldValue = GetSubstring(line, delStart, delEnd, false);

				delStart = ";";
				delEnd = ";;";
				auto fieldType = GetSubstring(line, delStart, delEnd, false);

				switch (std::stoi(fieldType))
				{
				case 1:
				{
					fields.intFields.emplace(fieldName, std::stoi(fieldValue));
					break;
				}
				case 2:
				{
					fields.floatFields.emplace(fieldName, std::stof(fieldValue));
					break;
				}
				case 3:
				{
					fields.stringFields.emplace(fieldName, fieldValue);
					break;
				}
				case 4:
				{
					fields.boolFields.emplace(fieldName, std::stoi(fieldValue));
					break;
				}
				}
				comp.fields = fields;
				line.erase(0, fieldPos + delField.length());

			}
			ent.components.push_back(comp);
		}
		entities.push_back(ent);
	}


	myFile.close();

	for (auto& e : entities)
	{
		Entity* ent = new Entity(e.entityName);
		for (auto& c : e.components)
		{
			if (c.componentName == "Transform")
			{
				ent->GetComponent<Transform>().InitSerializedFields(c.fields);
			}
		}
		manager->addEntity(ent);
	}
}

std::string Engine::GetSubstring(std::string& line, std::string& delStart, std::string& delEnd, bool erase = false)
{
	auto first = line.find(delStart);
	auto last = line.find(delEnd);
	SerializableComponent comp;
	auto final = line.substr(first + delStart.length(), last - (first + delStart.length()));
	if (erase)
		line.erase(first, last - first + delEnd.length());

	return final;
}

