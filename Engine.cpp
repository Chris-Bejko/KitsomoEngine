#include "Engine.h"
#include <iostream>
#include "SystemManager.h"
#include "Serialization.h"
#include "Transform.h"
#include "ComponentRegistry.h"
#include <cassert>
#include "imgui.h"
#include "imgui-sfml.h"
#include "imguiHandler.h"
#include <fstream>
#include "Timedelta.h"
#include "Logger.h"
#include "GizmoSystem.h"
#include "ProjectModuleLoader.h"
#include "UI/UIEventSystem.h"
#include "AssetManager.h"
#include "GUIDGenerator.h"
#include "nlohmann/json.hpp"
#include "PlayerPrefs.h"
#include "SceneManager.h"
#include "DialogManager.h"
#include "ConsoleManager.h"
#include "StatusManager.h"
#include "Sprite.h"
#include "EventSystem.h"

using json = nlohmann::json;

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

Engine &Engine::get()
{
	static Engine instance;
	return instance;
}

void Engine::Quit()
{
	isRunning = false;
}

void Engine::SubscribeEvents()
{
	SUBSCRIBE_EVENT(OpenProjectEvent, OpenProject);

	// EventSystem::get().Subscribe<OpenProjectEvent>([this](const OpenProjectEvent& event)
	// {
	// 	this->OpenProject(event.projectPath);
	// });
}
void Engine::Init()
{
	this->window = new sf::RenderWindow(sf::VideoMode(1280, 720), "SFML works!");
	window->setFramerateLimit(1000);
	if (!ImGui::SFML::Init(GetWindow()))
	{
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

    ConsoleManager::get().AddToConsole(message, level); });
	auto inputSystem = new InputSystem();
	this->inputSystem = inputSystem;
	SystemsManager::get().AddSystem(inputSystem);
	// To Do: Extract functions to make this a bit clearer.. should the engine be responsible for assigning dialogs, or the dialog system itself? Or the dialogs should autoregister themselves like components do? Hmm.
	manager = new EntityManager();
	projectModuleLoader = std::make_unique<ProjectModuleLoader>();
	SceneManager::get().Init();
	AssetManager::get().loadFont("dmPrison", "Assets/fonts/Domestic Prison.ttf");
	std::ofstream txtFile;
	PlayerPrefs::get().Load();
	RegisterComponents();
	SubscribeEvents();
	isRunning = true;
}

void Engine::Clean()
{
	AssetManager::get().clean();
	if (projectModuleLoader && projectModuleLoader->HasLoadedModule())
	{
		projectModuleLoader->StopWatching();
		manager->DestroyAllEntities();
		projectModuleLoader->UnloadProjectModule();
	}
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
	if (this->loading)
		return;
	ProcessHotReloading();
	ProcessDestroyQueue();
	bool checkBox;
	switch (currentState)
	{
	case EngineState::Running:
	{
		if (!isEngine)
		{
			manager->update(dt);
			manager->Collisions();
			break;
		}
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
		break;
	}
	}
	auto rest = deltaClock.restart();

	if (rest.asSeconds() <= 0.f)
	{
		rest = sf::seconds(1.f / 60.f);
	}
	dt = rest.asSeconds();

	// LOG_DEBUG("dt = ", dt, " | fps = ", 1.f / dt);
	Timedelta::deltaTime = dt;
	ImguiHandler::get().Update(rest);
	DialogManager::get().Update();
	ProcessDestroyQueue();
	SystemsManager::get().Update();
	StatusManager::get().Update(dt);
	UIEventSystem::get().Update();
}

void Engine::UpdateRuntime()
{
	if (!isRunning || manager == nullptr)
		return;

	auto rest = deltaClock.restart();

	if (rest.asSeconds() <= 0.f)
	{
		rest = sf::seconds(1.f / 60.f);
	}
	Timedelta::deltaTime = dt;

	dt = rest.asSeconds();

	manager->update(dt);
	manager->Collisions();
	ProcessDestroyQueue();
	SystemsManager::get().Update();
	UIEventSystem::get().Update();
}

void Engine::QueueDestroy(const std::string &guid)
{
	// Find entity
	Entity *e = nullptr;
	for (auto &ent : manager->GetEntities())
	{
		if (ent->GetGUID() == guid)
		{
			e = ent.get();
			break;
		}
	}
	if (!e)
		return;

	// Mark this entity
	e->SetPendingDestroy(true);
	destroyQueue.push_back(guid);

	// Also queue all children recursively
	QueueDestroyChildren(e);
}

void Engine::QueueDestroyChildren(Entity *e)
{
	for (auto *child : e->GetChildren())
	{
		if (!child || child->IsPendingDestroy())
			continue;
		child->SetPendingDestroy(true);
		destroyQueue.push_back(child->GetGUID());
		QueueDestroyChildren(child); // recurse
	}
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
		if (isEngine)
		{
			ImGui::SFML::ProcessEvent(event);
		}
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

void Engine::Draw(Sprite *sprite)
{
	if (window == nullptr || sprite == nullptr)
		return;

	const sf::Sprite sfmlSprite = sprite->GetSprite();

	if (!sfmlSprite.getTexture())
		return;

	window->draw(sfmlSprite);
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

void Engine::PrepareForProjectModuleUnload()
{
	// Unload the project module to prepare for recompilation
	manager->ClearAllEntities();
}
void Engine::ClearInspector()
{
	manager->ClearInspector();
	manager->SetSelectedEntity(nullptr);
	draggedEntity = "";
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

void Engine::Save(const std::string &filename)
{
	if (!SceneManager::get().HasProjectRoot())
	{
		LOG_WARNING("Cannot save without a loaded project");
		return;
	}

	std::filesystem::path path = SceneManager::get().ResolveProjectPath(filename);
	std::filesystem::create_directories(path.parent_path());

	json root = json::array();

	for (auto &e : manager->GetEntities())
	{
		json entityJson;
		entityJson["name"] = e->GetName();
		entityJson["guid"] = e->GetGUID();
		entityJson["parent"] = e->HasParent() ? e->GetParent()->GetGUID() : "";

		json componentsJson = json::array();
		for (auto &comp : e->GetAllComponentVariables())
		{
			json compJson;
			compJson["type"] = comp.componentName;
			compJson["guid"] = comp.guiD;

			json fieldsJson;
			for (auto &f : comp.variables)
			{
				switch (f.type)
				{
				case int_Type:
					fieldsJson[f.name] = *reinterpret_cast<int *>(f.data);
					break;
				case float_Type:
					fieldsJson[f.name] = *reinterpret_cast<float *>(f.data);
					break;
				case char_Type:
					fieldsJson[f.name] = *reinterpret_cast<std::string *>(f.data);
					break;
				case bool_Type:
					fieldsJson[f.name] = *reinterpret_cast<bool *>(f.data);
					break;
				case mathVector_Type:
				{
					std::string packed;
					if (f.isIntVector)
					{
						auto *data = reinterpret_cast<int *>(f.data);
						for (int i = 0; i < f.vectorSize; i++)
						{
							if (i > 0)
								packed += "|";
							packed += std::to_string(data[i]);
						}
					}
					else
					{
						auto *data = reinterpret_cast<float *>(f.data);
						for (int i = 0; i < f.vectorSize; i++)
						{
							if (i > 0)
								packed += "|";
							packed += std::to_string(data[i]);
						}
					}
					fieldsJson[f.name] = packed;
					break;
				}

				case entityRef_Type:
					fieldsJson[f.name] = *reinterpret_cast<std::string *>(f.data);
					break;
				case compRef_Type:
					fieldsJson[f.name] = *reinterpret_cast<std::string *>(f.data);
					break;
				case file_Type:
					fieldsJson[f.name] = *reinterpret_cast<std::string *>(f.data);
					break;
				}
			}
			compJson["fields"] = fieldsJson;
			componentsJson.push_back(compJson);
		}
		entityJson["components"] = componentsJson;
		root.push_back(entityJson);
	}

	std::ofstream file(path);
	file << root.dump(2); // 2 = indent spaces, makes it human readable
	file.close();
	LOG_INFO("Saved: ", path.string().c_str());
}

bool Engine::Load(std::string fileName)
{
	if (!SceneManager::get().HasProjectRoot())
	{
		LOG_WARNING(
			"Cannot load scene without a loaded project");

		return false;
	}

	if (fileName.empty())
	{
		return true;
	}

	// ------------------------------------------------------------
	// Scene files live under Assets/Scenes
	// ------------------------------------------------------------
	if (!isEngine)
		fileName += ".scene";
	std::filesystem::path path =
		SceneManager::get().GetSceneDirectory() /
		fileName;

	LOG_INFO(
		"Loading file: ",
		path.string().c_str());

	if (!std::filesystem::exists(path))
	{
		LOG_ERROR(
			"Scene file does not exist: ",
			path.string().c_str());

		this->loading = false;
		return false;
	}

	this->loading = true;

	auto entities = ParseFile(path.string());

	if (entities.empty())
	{
		this->loading = false;

		LOG_ERROR(
			"Scene contains no entities or failed to parse: ",
			path.string().c_str());

		return false;
	}

	GizmoSystem::get().SetSelectedEntity(nullptr);

	SpawnEntities(entities);

	currentState = EngineState::Running;

	openProject = fileName;

	this->loading = false;

	return true;
}
bool Engine::LoadPrefab(std::string prefabName)
{
	if (!SceneManager::get().HasProjectRoot())
	{
		LOG_WARNING("Cannot load prefab without a loaded project");
		return false;
	}

	std::filesystem::path path = SceneManager::get().ResolveProjectPath("Assets/Prefabs/" + prefabName + ".prefab");
	LOG_INFO("Loading prefab: ", path.string().c_str());

	auto entities = ParseFile(path.string());
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
	if (!SceneManager::get().HasProjectRoot())
	{
		LOG_WARNING("Cannot spawn prefab without a loaded project");
		return nullptr;
	}

	std::filesystem::path path = SceneManager::get().ResolveProjectPath("Assets/Prefabs/" + prefabName + ".prefab");
	auto entities = ParseFile(path.string());
	if (entities.empty())
	{
		LOG_WARNING("Prefab not found: ", path.c_str());
		return nullptr;
	}

	// Only spawn first entity from prefab
	auto &e = entities[0];
	e.entityName = manager->GetUniqueName(e.entityName);

	Entity *ent = new Entity(e.entityName, e.guiD);
	for (auto &c : e.components)
	{
		LOG_DEBUG("Spawning component: ", c.componentName.c_str());
		auto it = componentRegistry.find(c.componentName);
		if (it != componentRegistry.end())
			it->second(ent, c.fields, c.guiD);
	}

	// Override position
	ent->GetComponent<Transform>().position = position;
	Spawn(ent);
	ent->Awake();
	LOG_INFO("Spawned prefab with name'", ent->GetName().c_str(), "' at ", position.x, ", ", position.y);
	return ent;
}

void Engine::RegisterComponents()
{
	componentRegistry.clear();
	for (const auto &entry : ComponentRegistry::get().GetAll())
	{
		LOG_INFO("Registering component: ", entry.first.c_str());
		componentRegistry[entry.first] = entry.second.applySerialized;
	}
}

void Engine::OpenProject(const OpenProjectEvent &event)
{
	this->OpenProject(event.projectPath);
}

bool Engine::OpenProject(const std::string &projectPath)
{
	LOG_INFO(
		"OPEN PROJECT 1: BEGIN: ",
		projectPath);

	if (!projectModuleLoader)
	{
		LOG_ERROR(
			"OPEN PROJECT ERROR: projectModuleLoader is null");

		return false;
	}

	// ------------------------------------------------------------
	// Unload currently loaded project module
	// ------------------------------------------------------------
	if (projectModuleLoader->HasLoadedModule())
	{
		LOG_INFO(
			"OPEN PROJECT 2: Existing project module detected");

		projectModuleLoader->StopWatching();

		if (manager != nullptr)
		{
			LOG_INFO(
				"OPEN PROJECT 3: Destroying existing entities");

			manager->DestroyAllEntities();
		}

		LOG_INFO(
			"OPEN PROJECT 4: Unregistering old project components");

		ComponentRegistry::get()
			.UnregisterProjectComponents();

		LOG_INFO(
			"OPEN PROJECT 5: Unloading old project DLL");

		projectModuleLoader->UnloadProjectModule();

		LOG_INFO(
			"OPEN PROJECT 6: Re-registering engine components");

		RegisterComponents();
	}
	else
	{
		LOG_INFO(
			"OPEN PROJECT 2: No existing project module");
	}

	// ------------------------------------------------------------
	// Open project through SceneManager
	// ------------------------------------------------------------
	LOG_INFO(
		"OPEN PROJECT 7: Calling SceneManager::OpenProject");

	if (!SceneManager::get().OpenProject(projectPath))
	{
		LOG_ERROR(
			"OPEN PROJECT ERROR: SceneManager::OpenProject FAILED");

		EventSystem::get().Fire(ProjectLoadFailedEvent{});
		return false;
	}

	LOG_INFO(
		"OPEN PROJECT 8: SceneManager project opened");

	const auto projectRoot =
		SceneManager::get().GetProjectRootPath();

	LOG_INFO(
		"OPEN PROJECT 9: Resolved project root: ",
		projectRoot.string());

	// ------------------------------------------------------------
	// Load project module
	//
	// Editor:
	//     Build + load GameScripts
	//
	// Runtime:
	//     Load already compiled GameScripts.dll
	// ------------------------------------------------------------
	bool moduleLoaded = false;

	if (isEngine)
	{
		LOG_INFO(
			"OPEN PROJECT 10: Editor mode - building project DLL");

		moduleLoaded =
			projectModuleLoader->LoadProjectModule(
				projectRoot);

		if (!moduleLoaded)
		{
			LOG_ERROR(
				"OPEN PROJECT ERROR: LoadProjectModule FAILED");

			EventSystem::get().Fire(ProjectLoadFailedEvent{});
			return false;
		}

		LOG_INFO(
			"OPEN PROJECT 11: Project DLL built and loaded");
	}
	else
	{
		LOG_INFO(
			"OPEN PROJECT 10: Runtime mode - loading existing DLL");

		moduleLoaded =
			projectModuleLoader->LoadCompiledProjectModule(
				projectRoot);

		if (!moduleLoaded)
		{
			LOG_ERROR(
				"OPEN PROJECT ERROR: LoadCompiledProjectModule FAILED");

			EventSystem::get().Fire(ProjectLoadFailedEvent{});
			return false;
		}

		LOG_INFO(
			"OPEN PROJECT 11: Existing project DLL loaded");
	}

	// ------------------------------------------------------------
	// Register components exposed by project DLL
	// ------------------------------------------------------------
	LOG_INFO(
		"OPEN PROJECT 12: Calling RegisterComponents");

	RegisterComponents();

	LOG_INFO(
		"OPEN PROJECT 13: RegisterComponents finished");

	// ------------------------------------------------------------
	// Editor-only functionality
	// ------------------------------------------------------------
	if (isEngine)
	{
		LOG_INFO(
			"OPEN PROJECT 14: Starting script watcher");

		projectModuleLoader->StartWatching(
			SceneManager::get().GetScriptsDirectory());

		LOG_INFO(
			"OPEN PROJECT 15: Script watcher started");

		// Load an initial editor scene
		auto availableScenes =
			SceneManager::get().GetAvailableScenes();

		LOG_INFO(
			"OPEN PROJECT 16: Available scenes: ",
			availableScenes.size());

		if (std::find(
				availableScenes.begin(),
				availableScenes.end(),
				"MainNew") != availableScenes.end())
		{
			LOG_INFO(
				"OPEN PROJECT 17: Loading MainNew");

			SceneManager::get().LoadScene(
				"MainNew",
				SceneLoadMode::Replace);
		}
		else if (!availableScenes.empty())
		{
			LOG_INFO(
				"OPEN PROJECT 17: Loading first available scene");

			SceneManager::get().LoadScene(
				availableScenes.front(),
				SceneLoadMode::Replace);
		}
		else
		{
			LOG_WARNING(
				"OPEN PROJECT 17: No scenes available");
		}
	}
	else
	{
		// Runtime scene is loaded by InitRuntime()
		LOG_INFO(
			"OPEN PROJECT 14: Runtime mode - skipping editor scene loading");
	}

	LOG_INFO(
		"OPEN PROJECT 18: SUCCESS");

	EventSystem::get().Fire(ProjectLoadSuccessEvent{});

	return true;
}

bool Engine::ReloadProjectScripts()
{
	if (!SceneManager::get().HasProjectRoot())
	{
		return false;
	}

	pendingRecompile = true;
	const std::string activeScene = openProject;

	if (!projectModuleLoader->RebuildProjectModule(SceneManager::get().GetProjectRootPath()))
	{
		pendingRecompile = false;
		return false;
	}

	ClearInspector();
	GizmoSystem::get().SetSelectedEntity(nullptr);
	UIEventSystem::get().Clear();
	manager->DestroyAllEntities();
	RegisterComponents();

	if (!activeScene.empty())
	{
		Load(activeScene);
	}

	pendingRecompile = false;
	return true;
}

void Engine::RequestScriptRecompile()
{
	recompileRequested = true;
}

void Engine::ProcessHotReloading()
{
	if (!projectModuleLoader)
	{
		return;
	}

	if (projectModuleLoader->ConsumeReloadRequest())
	{
		recompileRequested = true;
	}

	if (!recompileRequested || currentState != EngineState::Running || loading)
	{
		return;
	}

	recompileRequested = false;
	ReloadProjectScripts();
}

void Engine::SavePrefab(Entity *entity)
{
	if (!SceneManager::get().HasProjectRoot())
	{
		LOG_WARNING("Cannot save prefab without a loaded project");
		return;
	}

	std::filesystem::create_directories(SceneManager::get().GetPrefabDirectory());

	json root = json::array();

	json entityJson;
	entityJson["name"] = entity->GetName();
	entityJson["parent"] = "";

	json componentsJson = json::array();
	for (auto &comp : entity->GetAllComponentVariables())
	{
		json compJson;
		compJson["type"] = comp.componentName;

		json fieldsJson;
		for (auto &f : comp.variables)
		{
			switch (f.type)
			{
			case int_Type:
				fieldsJson[f.name] = *reinterpret_cast<int *>(f.data);
				break;
			case float_Type:
				fieldsJson[f.name] = *reinterpret_cast<float *>(f.data);
				break;
			case char_Type:
			case entityRef_Type:
			case compRef_Type:
				fieldsJson[f.name] = *reinterpret_cast<std::string *>(f.data);
				break;
			case bool_Type:
				fieldsJson[f.name] = *reinterpret_cast<bool *>(f.data);
				break;
				// Save
			case mathVector_Type:
			{
				std::string packed;
				if (f.isIntVector)
				{
					auto *data = reinterpret_cast<int *>(f.data);
					for (int i = 0; i < f.vectorSize; i++)
					{
						if (i > 0)
							packed += "|";
						packed += std::to_string(data[i]);
					}
				}
				else
				{
					auto *data = reinterpret_cast<float *>(f.data);
					for (int i = 0; i < f.vectorSize; i++)
					{
						if (i > 0)
							packed += "|";
						packed += std::to_string(data[i]);
					}
				}
				fieldsJson[f.name] = packed;
				break;
			}
			case file_Type:
			{
				fieldsJson[f.name] = *reinterpret_cast<std::string *>(f.data);
				break;
			}
			}
		}
		compJson["fields"] = fieldsJson;
		componentsJson.push_back(compJson);
	}
	entityJson["components"] = componentsJson;
	root.push_back(entityJson); // push into array

	std::filesystem::path path = SceneManager::get().GetPrefabDirectory() / (entity->GetName() + ".prefab");
	std::filesystem::create_directories(path.parent_path());
	std::ofstream file(path);
	file << root.dump(2);
	file.close();
	LOG_INFO("Saved prefab: ", path.string().c_str());
}

void Engine::ProcessDestroyQueue()
{
	for (auto &guid : destroyQueue)
	{
		Entity *e = nullptr;
		for (auto &ent : manager->GetEntities())
		{
			if (ent->GetGUID() == guid)
			{
				e = ent.get();
				break;
			}
		}
		if (!e)
			continue;

		manager->RemoveCollisionPairsForEntity(e);

		Entity *parent = e->GetParent();
		if (parent)
		{
			auto &siblings = parent->GetChildren();
			siblings.erase(
				std::remove(siblings.begin(), siblings.end(), e),
				siblings.end());
			e->ForceNullParent();
		}

		auto childrenCopy = e->GetChildren();
		for (auto *child : childrenCopy)
			if (child)
				child->ForceNullParent();
		e->GetChildren().clear();

		if (e->transform)
			e->transform->SetParent(nullptr);
	}

	// Remove all queued entities AFTER all cleanup
	for (auto &guid : destroyQueue)
		manager->RemoveEntityByGUID(guid);

	destroyQueue.clear();
}

void Engine::Reset()
{
	ClearInspector();
	GizmoSystem::get().SetSelectedEntity(nullptr);
	UIEventSystem::get().Clear();

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
				if (e->GetGUID() == focusTargetName)
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
	std::ifstream file(fileName);
	if (!file.is_open())
	{
		LOG_ERROR("Failed to open: ", fileName.c_str());
		return entities;
	}

	json root;
	try
	{
		file >> root;
	}
	catch (const json::exception &e)
	{
		LOG_ERROR("JSON parse error: ", e.what());
		return entities;
	}

	// Handle both single object (prefab) and array (scene)
	json entityArray = root.is_array() ? root : json::array({root});

	for (auto &entityJson : root)
	{
		SerializableEntity ent;
		ent.entityName = entityJson.value("name", "");
		ent.guiD = entityJson.value("guid", "");
		ent.parentGUID = entityJson.value("parent", "");

		for (auto &compJson : entityJson["components"])
		{
			SerializableComponent comp;
			comp.componentName = compJson.value("type", "");
			comp.guiD = compJson.value("guid", "");

			if (compJson.contains("fields"))
			{
				for (auto &[key, val] : compJson["fields"].items())
				{
					if (val.is_boolean())
						comp.fields.boolFields[key] = val.get<bool>();
					else if (val.is_number_integer())
						comp.fields.intFields[key] = val.get<int>();
					else if (val.is_number())
						comp.fields.floatFields[key] = val.get<float>();
					else if (val.is_string())
						comp.fields.stringFields[key] = val.get<std::string>();
				}
			}
			ent.components.push_back(comp);
		}
		entities.push_back(ent);
	}

	return entities;
}

void Engine::SpawnEntities(const std::vector<SerializableEntity> &entities)
{
	// First pass - spawn all entities
	for (auto &e : entities)
	{
		Entity *ent = new Entity(e.entityName, e.guiD);
		for (auto &c : e.components)
		{
			auto it = componentRegistry.find(c.componentName);
			if (it != componentRegistry.end())
				it->second(ent, c.fields, c.guiD);
			else
				LOG_WARNING("Unknown component: ", c.componentName.c_str());
		}
		manager->addEntity(ent);
	}

	// Second pass - link parents after all entities exist
	manager->ValidateAdded();
	for (auto &e : entities)
	{
		if (e.parentGUID.empty())
			continue;

		Entity *child = nullptr;
		Entity *parent = nullptr;

		for (auto &ent : manager->GetEntities())
		{
			if (ent->GetGUID() == e.guiD)
				child = ent.get();
			if (ent->GetGUID() == e.parentGUID)
				parent = ent.get();
		}

		if (child && parent)
		{
			child->SetParent(parent);
			LOG_DEBUG("Parented '", e.guiD.c_str(), "' to '", e.parentGUID.c_str(), "'");
		}
	}
}
void Engine::FocusOnEntity(Entity *entity)
{
	focusTargetName = entity->GetGUID();

	LOG_INFO("Focusing on: ", focusTargetName.c_str());
}

void Engine::TriggerGameOver()
{
	isGameOver = true;
	LOG_INFO("GAME OVER!");
	// For now just stop play mode
	SetEngineState(EngineState::Running);
}

// Build:
void Engine::InitRuntime(
	const std::string &projectRoot,
	const std::string &startupScene)
{
	Logger::get().SetLogToFile(true, "runtime.log");
	LOG_INFO("RUNTIME 1: InitRuntime BEGIN");
	isEngine = false;
	isRunning = true;

	currentState = EngineState::PlayMode;
	previousState = EngineState::PlayMode;
	entitiesAwaken = false;
	loading = false;
	AssetManager::get().loadFont("dmPrison", "Assets/fonts/Domestic Prison.ttf");

	LOG_INFO("RUNTIME 2: Creating window");

	window = new sf::RenderWindow(
		sf::VideoMode(1280, 720),
		"Game");

	window->setFramerateLimit(60);

	LOG_INFO("RUNTIME 3: Creating input");

	auto inputSystem = new InputSystem();
	this->inputSystem = inputSystem;
	SystemsManager::get().AddSystem(inputSystem);

	LOG_INFO("RUNTIME 4: Creating EntityManager");

	manager = new EntityManager();
	projectModuleLoader =
		std::make_unique<ProjectModuleLoader>();
	LOG_INFO("RUNTIME 5: Initializing SceneManager");

	SceneManager::get().Init();

	LOG_INFO("RUNTIME 6: Opening project");

	if (!OpenProject(projectRoot))
	{
		LOG_ERROR(
			"Runtime failed to open project: ",
			projectRoot.c_str());

		isRunning = false;
		return;
	}

	LOG_INFO("RUNTIME 7: Project opened");

	LOG_INFO("RUNTIME 8: Components registered");

	LOG_INFO(
		"RUNTIME 9: Loading scene: ",
		startupScene.c_str());

	if (!Load(startupScene))
	{
		LOG_ERROR(
			"Runtime failed to load startup scene: ",
			startupScene.c_str());

		isRunning = false;
		return;
	}

	LOG_INFO("RUNTIME 10: Scene loaded");

	manager->Awake();

	LOG_INFO("RUNTIME 11: Entities awakened");

	entitiesAwaken = true;

	LOG_INFO("RUNTIME 12: InitRuntime COMPLETE");
}

void Engine::RenderRuntime()
{
	window->clear();

	manager->draw();

	window->display();
}