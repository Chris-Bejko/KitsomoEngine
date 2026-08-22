# KitsomoEngine

KitsomoEngine is a Unity-style game engine experiment written in C++ with an entity-component (OOP) approach. It started as a personal learning project and has grown into a small editor-style engine with a scene-based workflow, custom editor tools, and a modular component system.

The goal of the project is to explore what it feels like to design, architect and build a lightweight engine with familiar concepts such as entities, components, scenes, prefabs, and an in-editor workflow without needing a full production pipeline.

## Inspiration

This project was also heavily influenced by the game I made as a remake of [Color Bullets](https://kitsomo.itch.io/color-bullets). That game became a practical guide for which engine features were worth building next, especially around scene editing, gameplay tooling, object interaction, and editor usability.

## What it includes

- Unity-inspired editor layout with panels for entities, inspector, console, and scenes
- Entity-component style architecture for organizing gameplay logic and data
- Scene management with save/load support and scene-based editing
- Project-style workflow with folder-based project organization
- Prefab creation and prefab loading
- Inspector-driven component editing
- Transform tools and gizmo support for scene manipulation
- Drag-and-drop interaction for entities and scene objects
- Command-based editing with undo/redo support
- Collision and physics-related components
- Audio support through SFML
- Input handling and camera systems
- Basic gameplay UI elements for editor-driven playtesting
- Serialization support for scenes, entities, and editor data

## What it does not include, but might be added  
- Proper File explorer navigation in-engine -> Currently being done on Build Pipeline branch
- A scripting language & Hot reloading of it -> No scripting language yet, but Hot Reloading is officially a thing, though it can probably be better
- Unity-style building  (from inside the engine) -> Build Pipeline branch exists to provide such support, very near to finished
- Cursor state update -> Blocking events like hot reloading now just freeze the game. It would be nice to be able to set a "Loading" cursor. imgui/sfml probably has something there. 

## What can be improved   
I am aware that potential employers and recruiters might be looking at this page, and the code is clearly not perfect.
- One thing that bothers me is how I approached drawing all the ImGui windows. I made one file , imguiHandler.cpp, and just started adding to it. This could be refactored to feature a more abstract and object oriented approach, making it easy for users to also make their own dialogs easily. -> Almost fixed, dialogs are separate now, but they are blocked on editor game mode :D.
- Entities is responsible for drawing the serialized components on the inspector. This was deliberate but there is probably a better way.. looking into it.
- 
- To be continued (But if you are a recruiter, feel free to ask me about my design decisions :).)

## Tech stack

- C++17
- SFML for rendering, windowing, input, and audio
- ImGui for the editor interface
- CMake for building

## Build

The project is built with CMake. From the project root, you can generate and build it with:

```bash
cmake -S . -B build
cmake --build build
```

## Notes

This engine is still experimental and evolving. It is being developed as a learning project and a playground for editor workflows, OOP design, gameplay tooling, and engine architecture ideas.
