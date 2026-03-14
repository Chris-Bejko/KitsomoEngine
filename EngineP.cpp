#include "Engine.h"
#include <iostream>
#include "SystemManager.h"
#include "Serialization.h"
#include "Components/Transform.h"
#include <cassert>
#include "imgui.h"
#include "imgui-sfml.h"
#include "imguiHandler.h"
#include <fstream>
#include "Components/Sprite.h"
#include "Components/Rigidbody.h"
#include "Components/Player.h"
#include "Components/FloorSquare.h"
#include "Timedelta.h"
#include "Logger.h"
Engine *Engine::s_instance = nullptr;

Engine::Engine()
{
	isRunning = false;
	isEngine = true;
	currentState = EngineState::Running;
	previousState = EngineState::Paused;
	entitiesAwaken = false;
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
	window->setFramerateLimit(1000);
	if (!ImGui::SFML::Init(GetWindow()))
	{
		// std::cerr << "Error initializing IMGUI window" << std::endl;
		LOG_ERROR("Error initializing IMGUI window");
	}
	else
	{
		LOG_INFO("Window initialized");
	}
	ImguiHandler::ApplyEditorStyle();
	Logger::get().SetCallback([](LogLevel level, const std::string &message)
							  {
    ImVec4 color;
    switch (level)
    {
    case LogLevel::Info:    color = ImVec4(0.9f, 0.9f, 0.9f, 1.0f); break;
    case LogLevel::Warning: color = ImVec4(0.95f, 0.78f, 0.2f, 1.0f); break;
    case LogLevel::Error:   color = ImVec4(1.0f, 0.3f, 0.3f, 1.0f); break;
    case LogLevel::Debug:   color = ImVec4(0.4f, 0.85f, 1.0f, 1.0f); break;
    default:                color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); break;
    }
    ImguiHandler::get().AddConsoleLog(message, color); });
	auto inputSystem = new InputSystem();
	this->inputSystem = inputSystem;
	SystemsManager::get().AddSystem(inputSystem);
	manager = new EntityManager();
	std::ofstream txtFile;

	// Load();
	// Entity* newEntity = new Entity("Player");
	// Entity* floorSquare = new Entity("floor Square");
	// Entity* floorSquare1 = new Entity("floor Square(1)");
	// Entity* floorSquare2 = new Entity("floor Square(2)");
	// Entity* floorSquare3 = new Entity("floorSquare(3)");
	// newEntity->AddComponent<Player>(true, Vector2F(100, 100), "player");
	// floorSquare->AddComponent<FloorSquare>().Config(Vector2F(250, 100), sf::Color(0, 128, 0, 255));
	// floorSquare1->AddComponent<FloorSquare>().Config(Vector2F(550, 159), sf::Color::Red);
	// floorSquare2->AddComponent<FloorSquare>().Config(Vector2F(1000, 500), sf::Color::Magenta);
	// floorSquare3->AddComponent<FloorSquare>().Config(Vector2F(500, 500), sf::Color::Cyan);
	// manager->addEntity(newEntity);
	// manager->addEntity(floorSquare);
	// manager->addEntity(floorSquare1);
	// manager->addEntity(floorSquare2);
	// manager->addEntity(floorSquare3);
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
	switch (currentState)
	{
	case EngineState::Running:
	{
		manager->updateEngine(dt);
		UpdateEditorCamera(dt); 
		break;
	}
	case EngineState::Paused:
	{
		manager->updateEngine(dt);
		UpdateEditorCamera(dt); 
		break;
	}
	case EngineState::PlayMode:
	{
		LOG_DEBUG("entitiesAwaken: ", entitiesAwaken, " entities count: ", manager->GetTotalEntities());

		if (!entitiesAwaken)
			break;

		manager->update(dt);
		manager->Collisions();
		break;
	}
	}
	SystemsManager::get().Update();
	auto rest = deltaClock.restart();

	if (rest.asSeconds() <= 0.f)
	{
		rest = sf::seconds(1.f / 60.f);
	}
	dt = rest.asSeconds();

	// LOG_DEBUG("dt = ", dt, " | fps = ", 1.f / dt);
	Timedelta::deltaTime = dt;
	ImguiHandler::get().Update(rest);
}

void Engine::Events()
{
	sf::Event event;

	while (window->pollEvent(event))
	{
		if (event.type == sf::Event::Closed)
			Clean();

		ImGui::SFML::ProcessEvent(event);
	}
}

EngineState Engine::GetCurrentState()
{
	return currentState;
}

void Engine::SetEngineState(EngineState engineState)
{
	previousState = currentState;
	currentState = engineState;
	isEngine = (engineState != EngineState::PlayMode);
	switch (engineState)
	{
	case EngineState::Running:
	{
		if (previousState != EngineState::Running)
		{
			Reset();
			entitiesAwaken = false;
		}
		break;
	}
	case EngineState::Paused:
	{
		break;
	}
	case EngineState::PlayMode:
	{
		// manager->ValidateAdded();
		if (!entitiesAwaken)
		{
			manager->Awake();
			entitiesAwaken = true;
		}
		break;
	}
	}
}

bool Engine::IsRunning()
{
	return isRunning;
}

sf::RenderWindow &Engine::GetWindow()
{
	return *window;
}

EntityManager *Engine::GetManager()
{
	return manager;
}

bool Engine::DraggingEntity()
{
	return draggedEntity != "";
}

std::string Engine::GetDraggedEntity()
{
	return draggedEntity;
}

void Engine::TriggerDragging(std::string newDragged)
{
	draggedEntity = newDragged;
}

void Engine::ClearInpsector()
{
	manager->ClearInspector();
}

void Engine::SetView(sf::View &view)
{
	window->setView(view);
}

sf::FloatRect Engine::GetView()
{
	sf::FloatRect rt;
	sf::View view(window->getView());
	rt.left = view.getCenter().x - view.getSize().x / 2.f;
	rt.top = view.getCenter().y - view.getSize().y / 2.f;
	rt.width = view.getCenter().x + view.getSize().x / 2.f;
	rt.height = view.getCenter().y + view.getSize().y / 2.f;

	return rt;
}

void Engine::Spawn(Entity *entity)
{
	manager->addEntity(entity);
}

size_t Engine::GetTotalEntities()
{
	return manager->GetTotalEntities();
}

void Engine::Save(std::string filename)
{
	auto entitiesAndComps = manager->SerializeEntities();
	std::ofstream myFile;
	myFile.open(filename);
	for (auto &e : entitiesAndComps)
	{
		myFile << "ENTITY_NAME_" << e.entityName << "_ENTITY_NAME_E_";
		for (auto &c : e.components)
		{
			myFile << "_COMPONENT_NAME_" << c.componentName;
			myFile << "_COMPONENT_NAME_E_";
			for (auto &f : c.variables)
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
					std::string p = *reinterpret_cast<std::string *>(f.data);
					myFile << "," << p << ",,";
					break;
				}
				case 4:
				{
					bool p = *reinterpret_cast<bool *>(f.data);
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

bool Engine::Load(std::string fileName)
{

	LOG_INFO("Loading file: ", fileName);
	if (fileName == "")
	{
		Init();
		return true;
	}
	// convert file to std::vector<SerializableEntity>();
	std::vector<SerializableEntity> entities;
	std::vector<SerializableComponent> components;

	std::fstream myFile;
	std::string line;
	myFile.open(fileName, std::ios::out | std::ios::in);
	if (!bool(myFile))
	{
		return false;
	}
	std::string entityName;
	while (std::getline(myFile, line))
	{
		SerializableEntity ent;
		std::string delStart = "ENTITY_NAME_";
		std::string delEnd = "_ENTITY_NAME_E_";
		ent.entityName = GetSubstring(line, delStart, delEnd, true);
		std::size_t pos = 0;
		std::string delimiter = "_COMPONENT_NAME_";
		while ((pos = line.find(delimiter)) != std::string::npos)
		{
			ReadableSerializableVariableMap fields;
			delStart = "_COMPONENT_NAME_";
			delEnd = "_COMPONENT_NAME_E_";
			SerializableComponent comp;
			comp.componentName = GetSubstring(line, delStart, delEnd, true);

			std::size_t fieldPos = 0;
			std::string delField = "_FIELD_";
			std::string delFIelds = "_FIELDS_";
			while ((fieldPos = line.find(delField)) != std::string::npos && (fieldPos < line.find(delimiter)))
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

				if (fieldType == "_FIELD_")
					break;
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

	for (auto &e : entities)
	{
		Entity *ent = new Entity(e.entityName);
		for (auto &c : e.components)
		{
			if (c.componentName == "Transform")
			{
				ent->GetComponent<Transform>().InitSerializedFields(c.fields);
			}
			if (c.componentName == "BoxCollider")
			{
				if (!ent->HasComponent<BoxCollider>())
					ent->AddComponent<BoxCollider>().InitSerializedFields(c.fields);
			}
			if (c.componentName == "Sprite")
			{
				if (!ent->HasComponent<Sprite>())
					ent->AddComponent<Sprite>().InitSerializedFields(c.fields);
			}
			if (c.componentName == "Rigidbody")
			{
				// ent->GetComponent<Rigidbody>().InitSerializedFields(c.fields);
			}
			if (c.componentName == "Player")
			{
				if (!ent->HasComponent<Player>())
					ent->AddComponent<Player>().InitSerializedFields(c.fields);
			}
			if (c.componentName == "FloorSquare")
			{
				if (!ent->HasComponent<FloorSquare>())
					ent->AddComponent<FloorSquare>().InitSerializedFields(c.fields);
			}
		}
		manager->addEntity(ent);
	}
	currentState = EngineState::Running;
	return true;
}

std::string Engine::GetSubstring(std::string &line, std::string &delStart, std::string &delEnd, bool erase = false)
{
	auto first = line.find(delStart);
	auto last = line.find(delEnd);
	SerializableComponent comp;
	auto final = line.substr(first + delStart.length(), last - (first + delStart.length()));
	if (erase)
		line.erase(first, last - first + delEnd.length());

	return final;
}

void Engine::SavePrefab(Entity *entity)
{
	// Create prefabs directory if it doesn't exist
	std::filesystem::create_directories("prefabs");

	std::ofstream myFile;
	std::string name = entity->GetName();
	name = name.c_str();
	std::string filename = "prefabs/" + name + ".prefab";
	myFile.open(filename);
	LOG_INFO("Saving prefab to: ", std::filesystem::absolute(filename).string().c_str());
	auto components = entity->GetAllComponentVariables();
	myFile << "ENTITY_NAME_" << entity->GetName() << "_ENTITY_NAME_E_";
	for (auto &c : components)
	{
		myFile << "_COMPONENT_NAME_" << c.componentName;
		myFile << "_COMPONENT_NAME_E_";
		for (auto &f : c.variables)
		{
			myFile << "_FIELD_";
			myFile << ":" << f.name << "::";
			switch (f.type)
			{
			case 1:
				myFile << "," << f.read() << ",,";
				break;
			case 2:
				myFile << "," << f.read() << ",,";
				break;
			case 3:
			{
				std::string p = *reinterpret_cast<std::string *>(f.data);
				myFile << "," << p << ",,";
				break;
			}
			case 4:
			{
				bool p = *reinterpret_cast<bool *>(f.data);
				myFile << "," << p << ",,";
				break;
			}
			}
			myFile << ";" << f.type << ";;";
		}
		myFile << "_FIELD_";
	}
	myFile << "\n";
	myFile.close();
	LOG_INFO("Saved prefab: ", filename);
}

bool Engine::LoadPrefab(std::string prefabName)
{
	std::string path = "prefabs/" + prefabName + ".prefab";
	LOG_INFO("Looking for prefab at: ", std::filesystem::absolute(path).string().c_str());
	ImguiHandler::get().Notify("Loading prefab: " + prefabName, ImVec4(0.9f, 0.9f, 0.9f, 1.0f), 1.5f);
	std::fstream myFile;
	myFile.open(path, std::ios::in);
	if (!myFile)
	{
		LOG_WARNING("Prefab not found: ", path.c_str());
		return false;
	}

	std::string line;
	std::getline(myFile, line);
	myFile.close();

	// Reuse existing parsing logic
	SerializableEntity ent;
	std::string delStart = "ENTITY_NAME_";
	std::string delEnd = "_ENTITY_NAME_E_";
	ent.entityName = GetSubstring(line, delStart, delEnd, true);

	std::size_t pos = 0;
	std::string delimiter = "_COMPONENT_NAME_";
	while ((pos = line.find(delimiter)) != std::string::npos)
	{
		ReadableSerializableVariableMap fields;
		delStart = "_COMPONENT_NAME_";
		delEnd = "_COMPONENT_NAME_E_";
		SerializableComponent comp;
		comp.componentName = GetSubstring(line, delStart, delEnd, true);

		std::size_t fieldPos = 0;
		std::string delField = "_FIELD_";
		while ((fieldPos = line.find(delField)) != std::string::npos && (fieldPos < line.find(delimiter)))
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

			if (fieldType == "_FIELD_")
				break;
			switch (std::stoi(fieldType))
			{
			case 1:
				fields.intFields.emplace(fieldName, std::stoi(fieldValue));
				break;
			case 2:
				fields.floatFields.emplace(fieldName, std::stof(fieldValue));
				break;
			case 3:
				fields.stringFields.emplace(fieldName, fieldValue);
				break;
			case 4:
				fields.boolFields.emplace(fieldName, std::stoi(fieldValue));
				break;
			}
			comp.fields = fields;
			line.erase(0, fieldPos + delField.length());
		}
		ent.components.push_back(comp);
	}

	// Spawn the entity
	Entity *entity = new Entity(manager->GetUniqueName(ent.entityName));
	for (auto &c : ent.components)
	{
		if (c.componentName == "Transform")
			entity->GetComponent<Transform>().InitSerializedFields(c.fields);
		if (c.componentName == "BoxCollider" && !entity->HasComponent<BoxCollider>())
			entity->AddComponent<BoxCollider>().InitSerializedFields(c.fields);
		if (c.componentName == "Sprite" && !entity->HasComponent<Sprite>())
			entity->AddComponent<Sprite>().InitSerializedFields(c.fields);
		if (c.componentName == "Player" && !entity->HasComponent<Player>())
			entity->AddComponent<Player>().InitSerializedFields(c.fields);
		if (c.componentName == "FloorSquare" && !entity->HasComponent<FloorSquare>())
			entity->AddComponent<FloorSquare>().InitSerializedFields(c.fields);
	}
	manager->addEntity(entity);
	LOG_INFO("Loaded prefab: ", entity->GetName().c_str());

	return true;
}

void Engine::Reset()
{
	manager->DestroyAllEntities();
	sf::View view;
	view.setSize(window->getSize().x, window->getSize().y);
	window->setView(view);
	isEngine = true;
}

void Engine::RemoveEntity(Entity *entity)
{
	manager->eraseEntity(entity);
}

void Engine::UpdateEditorCamera(float dt)
{
    float speed = 300.f * dt;
    sf::View view = window->getView();

    // Arrow keys always move camera in editor
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))  view.move(-speed, 0);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) view.move(speed, 0);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))    view.move(0, -speed);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))  view.move(0, speed);

    // Mouse drag when not dragging an entity
    if (sf::Mouse::isButtonPressed(sf::Mouse::Middle))
    {
        sf::Vector2f mousePos = (sf::Vector2f)sf::Mouse::getPosition(*window);
        if (!editorDragging)
        {
            editorDragStart = mousePos;
            editorDragging = true;
        }
        sf::Vector2f delta = editorDragStart - mousePos;
        view.move(delta * 0.5f);
        editorDragStart = mousePos;
    }
    else
    {
        editorDragging = false;
    }

    // Left mouse drag when not dragging an entity
    if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && !DraggingEntity())
    {
        sf::Vector2f mousePos = (sf::Vector2f)sf::Mouse::getPosition(*window);
        if (!editorDragging)
        {
            editorDragStart = mousePos;
            editorDragging = true;
        }
        sf::Vector2f delta = editorDragStart - mousePos;
        view.move(delta);
        editorDragStart = mousePos;
    }
    else if (!sf::Mouse::isButtonPressed(sf::Mouse::Middle))
    {
        editorDragging = false;
    }

    window->setView(view);
}