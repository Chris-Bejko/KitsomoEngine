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
#include "Components/Bullet.h"
#include "Components/Rigidbody.h"
#include "Components/CircleCollider.h"
#include "Components/BoxCollider.h"
#include "Components/PolygonCollider.h"
#include "Components/AudioSource.h"
#include "Components/UIButton.h"
#include "Components/UIImage.h"
#include "Components/UIText.h"
#include "Components/Canvas.h"
#include "GizmoSystem.h"
#include "UI/UIEventSystem.h"
#include "AssetManager.h"

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
	AssetManager::get().loadFont("dmPrison", "fonts/Domestic Prison.ttf");
	std::ofstream txtFile;
	RegisterComponents();
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
	GizmoSystem::get().Draw();
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
		GizmoSystem::get().Update(dt);
		break;
	}
	case EngineState::Paused:
	{
		manager->updateEngine(dt);
		GizmoSystem::get().Update(dt);
		UpdateEditorCamera(dt);
		break;
	}
	case EngineState::PlayMode:
	{
		if (!entitiesAwaken)
			break;

		manager->update(dt);
		manager->Collisions();
		UIEventSystem::get().Update();

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

		if (event.type == sf::Event::MouseWheelScrolled &&
			sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))
		{
			if (currentState == EngineState::Running || currentState == EngineState::Paused)
			{
				sf::View view = window->getView();
				float zoomFactor = event.mouseWheelScroll.delta > 0 ? 0.9f : 1.1f;
				view.zoom(zoomFactor);
				window->setView(view);
			}
		}

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
	if (manager->IsInColliderEditMode())
		return;
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
	std::ofstream myFile;
	myFile.open(filename);

	for (auto &e : manager->GetEntities()) // iterate Entity* directly
	{
		myFile << "ENTITY_NAME_" << e->GetName() << "_ENTITY_NAME_E_";

		if (e->HasParent())
			myFile << "_PARENT_" << e->GetParent()->GetName() << "_PARENT_E_";

		auto components = e->GetAllComponentVariables();
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
	}
	myFile.close();
}
bool Engine::Load(std::string fileName)
{
	LOG_INFO("Loading file: ", fileName.c_str());
	if (fileName.empty())
		return true;

	auto entities = ParseFile(fileName);
	if (entities.empty())
		return false;
	GizmoSystem::get().SetSelectedEntity(nullptr);
	SpawnEntities(entities);
	currentState = EngineState::Running;
	openProject = fileName;
	return true;
}

bool Engine::LoadPrefab(std::string prefabName)
{
	std::string path = "prefabs/" + prefabName + ".prefab";
	LOG_INFO("Loading prefab: ", path.c_str());

	auto entities = ParseFile(path);
	if (entities.empty())
	{
		LOG_WARNING("Prefab not found: ", path.c_str());
		return false;
	}

	// Rename to avoid conflicts
	for (auto &e : entities)
		e.entityName = manager->GetUniqueName(e.entityName);

	SpawnEntities(entities);
	return true;
}

Entity *Engine::SpawnPrefab(const std::string prefabName, Vector2F position)
{
	std::string path = "prefabs/" + prefabName + ".prefab";
	auto entities = ParseFile(path);
	if (entities.empty())
	{
		LOG_WARNING("Prefab not found: ", path.c_str());
		return nullptr;
	}

	// Only spawn first entity from prefab
	auto &e = entities[0];
	e.entityName = manager->GetUniqueName(e.entityName);

	Entity *ent = new Entity(e.entityName);
	for (auto &c : e.components)
	{
		LOG_DEBUG("Spawning component: ", c.componentName.c_str());
		auto it = componentRegistry.find(c.componentName);
		if (it != componentRegistry.end())
			it->second(ent, c.fields);
	}

	// Override position
	ent->GetComponent<Transform>().position = position;

	LOG_INFO("Spawned prefab '", prefabName.c_str(), "' at ", position.x, ", ", position.y);
	return ent;
}

void Engine::RegisterComponents()
{
	componentRegistry["Transform"] = [](Entity *e, ReadableSerializableVariableMap fields)
	{
		e->GetComponent<Transform>().InitSerializedFields(fields);
	};
	componentRegistry["BoxCollider"] = [](Entity *e, ReadableSerializableVariableMap fields)
	{
		if (!e->HasComponent<BoxCollider>())
			e->AddComponent<BoxCollider>().InitSerializedFields(fields);
		else
			e->GetComponent<BoxCollider>().InitSerializedFields(fields);
	};
	componentRegistry["Sprite"] = [](Entity *e, ReadableSerializableVariableMap fields)
	{
		if (!e->HasComponent<Sprite>())
			e->AddComponent<Sprite>().InitSerializedFields(fields);
		else
			e->GetComponent<Sprite>().InitSerializedFields(fields);
	};
	componentRegistry["Player"] = [](Entity *e, ReadableSerializableVariableMap fields)
	{
		if (!e->HasComponent<Player>())
			e->AddComponent<Player>().InitSerializedFields(fields);
		else
			e->GetComponent<Player>().InitSerializedFields(fields);
	};
	componentRegistry["FloorSquare"] = [](Entity *e, ReadableSerializableVariableMap fields)
	{
		if (!e->HasComponent<FloorSquare>())
			e->AddComponent<FloorSquare>().InitSerializedFields(fields);
		else
			e->GetComponent<FloorSquare>().InitSerializedFields(fields);
	};
	componentRegistry["Bullet"] = [](Entity *e, ReadableSerializableVariableMap fields)
	{
		if (!e->HasComponent<Bullet>())
			e->AddComponent<Bullet>().InitSerializedFields(fields);
		else
			e->GetComponent<Bullet>().InitSerializedFields(fields);
	};
	componentRegistry["Rigidbody"] = [](Entity *e, ReadableSerializableVariableMap fields)
	{
		if (!e->HasComponent<Rigidbody>())
			e->AddComponent<Rigidbody>().InitSerializedFields(fields);
		else
			e->GetComponent<Rigidbody>().InitSerializedFields(fields);
	};
	componentRegistry["CircleCollider"] = [](Entity *e, ReadableSerializableVariableMap fields)
	{
		if (!e->HasComponent<CircleCollider>())
			e->AddComponent<CircleCollider>().InitSerializedFields(fields);
		else
			e->GetComponent<CircleCollider>().InitSerializedFields(fields);
	};
	componentRegistry["PolygonCollider"] = [](Entity *e, ReadableSerializableVariableMap fields)
	{
		if (!e->HasComponent<PolygonCollider>())
			e->AddComponent<PolygonCollider>().InitSerializedFields(fields);
		else
			e->GetComponent<PolygonCollider>().InitSerializedFields(fields);
	};
	componentRegistry["AudioSource"] = [](Entity *e, ReadableSerializableVariableMap fields)
	{
		if (!e->HasComponent<AudioSource>())
			e->AddComponent<AudioSource>().InitSerializedFields(fields);
		else
			e->GetComponent<AudioSource>().InitSerializedFields(fields);
	};
	componentRegistry["UIButton"] = [](Entity *e, ReadableSerializableVariableMap fields)
	{
		if (!e->HasComponent<UIButton>())
			e->AddComponent<UIButton>().InitSerializedFields(fields);
		else
			e->GetComponent<UIButton>().InitSerializedFields(fields);
	};
	componentRegistry["UIImage"] = [](Entity *e, ReadableSerializableVariableMap fields)
	{
		if (!e->HasComponent<UIImage>())
			e->AddComponent<UIImage>().InitSerializedFields(fields);
		else
			e->GetComponent<UIImage>().InitSerializedFields(fields);
	};
	componentRegistry["UIText"] = [](Entity *e, ReadableSerializableVariableMap fields)
	{
		if (!e->HasComponent<UIText>())
			e->AddComponent<UIText>().InitSerializedFields(fields);
		else
			e->GetComponent<UIText>().InitSerializedFields(fields);
	};
		componentRegistry["Canvas"] = [](Entity *e, ReadableSerializableVariableMap fields)
	{
		if (!e->HasComponent<Canvas>())
			e->AddComponent<Canvas>().InitSerializedFields(fields);
		else
			e->GetComponent<Canvas>().InitSerializedFields(fields);
	};
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

void Engine::Reset()
{
	manager->DestroyAllEntities();
	UIEventSystem::get().Clear();
	GizmoSystem::get().SetSelectedEntity(nullptr);
	sf::View view;
	view.setSize(window->getSize().x, window->getSize().y);
	window->setView(view);
	isEngine = true;
	if (openProject != "")
	{
		if (!Load(openProject))
		{
			LOG_ERROR("Failed to reload project: ", openProject.c_str());
		}
		return;
	}
}

void Engine::RemoveEntity(Entity *entity)
{
	manager->eraseEntity(entity);
}

void Engine::UpdateEditorCamera(float dt)
{
	sf::View view = window->getView();

	// Cancel focus if user moves manually
	if (!focusTargetName.empty())
	{
		bool userInput = sf::Keyboard::isKeyPressed(sf::Keyboard::Left) ||
						 sf::Keyboard::isKeyPressed(sf::Keyboard::Right) ||
						 sf::Keyboard::isKeyPressed(sf::Keyboard::Up) ||
						 sf::Keyboard::isKeyPressed(sf::Keyboard::Down) ||
						 sf::Mouse::isButtonPressed(sf::Mouse::Middle) ||
						 DraggingEntity();

		if (userInput)
		{
			focusTargetName = "";
		}
		else
		{
			Entity *target = nullptr;
			for (auto &e : manager->GetEntities())
			{
				if (e->GetName() == focusTargetName)
				{
					target = e.get();
					break;
				}
			}

			if (target == nullptr)
			{
				focusTargetName = "";
			}
			else
			{
				// Use world position instead of local position
				Vector2F worldPos = target->transform->GetWorldPosition();
				sf::Vector2f targetPos(worldPos.x, worldPos.y);

				sf::Vector2f currentCenter = view.getCenter();
				sf::Vector2f diff = targetPos - currentCenter;
				float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);

				if (dist < 1.f)
					focusTargetName = "";
				else
					view.setCenter(currentCenter + diff * focusSpeed * dt);

				window->setView(view);
				return;
			}
		}
	}
	float speed = 300.f * dt;

	// Arrow keys always move camera in editor
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
		view.move(-speed, 0);
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
		view.move(speed, 0);
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
		view.move(0, -speed);
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
		view.move(0, speed);

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

std::vector<SerializableEntity> Engine::ParseFile(const std::string &fileName)
{
	std::vector<SerializableEntity> entities;
	std::fstream myFile;
	myFile.open(fileName, std::ios::in);
	if (!myFile)
		return entities;

	std::string line;
	while (std::getline(myFile, line))
	{
		SerializableEntity ent;
		std::string delStart = "ENTITY_NAME_";
		std::string delEnd = "_ENTITY_NAME_E_";
		ent.entityName = GetSubstring(line, delStart, delEnd, true);
		if (line.find("_PARENT_") != std::string::npos)
		{
			delStart = "_PARENT_";
			delEnd = "_PARENT_E_";
			ent.parentName = GetSubstring(line, delStart, delEnd, true);
		}
		std::string delimiter = "_COMPONENT_NAME_";
		std::size_t pos = 0;
		while ((pos = line.find(delimiter)) != std::string::npos)
		{
			ReadableSerializableVariableMap fields;
			delStart = "_COMPONENT_NAME_";
			delEnd = "_COMPONENT_NAME_E_";
			SerializableComponent comp;
			comp.componentName = GetSubstring(line, delStart, delEnd, true);

			std::string delField = "_FIELD_";
			std::size_t fieldPos = 0;
			while ((fieldPos = line.find(delField)) != std::string::npos &&
				   (fieldPos < line.find(delimiter)))
			{
				delStart = ":";
				delEnd = "::";
				auto fieldName = GetSubstring(line, delStart, delEnd, false);
				delStart = ",";
				delEnd = ",,";
				auto fieldValue = GetSubstring(line, delStart, delEnd, false);

				// Find the position of the value end delimiter (,,)
				size_t valueEndPos = line.find(",,");
				// Search for field type AFTER the value ends
				std::string remainingLine = line.substr(valueEndPos);
				delStart = ";";
				delEnd = ";;";
				auto fieldType = GetSubstring(remainingLine, delStart, delEnd, false);

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
		entities.push_back(ent);
	}
	myFile.close();
	return entities;
}

void Engine::SpawnEntities(const std::vector<SerializableEntity> &entities)
{
	// First pass - spawn all entities
	for (auto &e : entities)
	{
		Entity *ent = new Entity(e.entityName);
		for (auto &c : e.components)
		{
			auto it = componentRegistry.find(c.componentName);
			if (it != componentRegistry.end())
				it->second(ent, c.fields);
			else
				LOG_WARNING("Unknown component: ", c.componentName.c_str());
		}
		manager->addEntity(ent);
	}

	// Second pass - link parents after all entities exist
	manager->ValidateAdded();
	for (auto &e : entities)
	{
		if (e.parentName.empty())
			continue;

		Entity *child = nullptr;
		Entity *parent = nullptr;

		for (auto &ent : manager->GetEntities())
		{
			if (ent->GetName() == e.entityName)
				child = ent.get();
			if (ent->GetName() == e.parentName)
				parent = ent.get();
		}

		if (child && parent)
		{
			child->SetParent(parent);
			LOG_DEBUG("Parented '", e.entityName.c_str(), "' to '", e.parentName.c_str(), "'");
		}
	}
}
void Engine::FocusOnEntity(Entity *entity)
{
	focusTargetName = entity->GetName();

	LOG_INFO("Focusing on: ", focusTargetName.c_str());
}