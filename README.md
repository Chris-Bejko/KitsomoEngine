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
- In-Engine event handling
- User defined Actions (Unity-Style events)
- File Explorer and Project loading
- Hot reloading and fully building a Project
## What it does not include, but might be added  
- A scripting language
- Cursor state update -> Blocking events like hot reloading now just freeze the game. It would be nice to be able to set a "Loading" cursor. imgui/sfml probably has something there.
- Scene architecture is currently poor, barely supported. 
  
##Improvements made so far
Dialogs now handle their own logic, deriving from a parent class and getting their draw called by the DialogManager
Serialization/Deserialization was improved by getting rid of enum-type handling, and replaced with class implementations instead. 
Entity is now fully decoupled from Imgui
Incremental build and unit tests added, now requirement for merging a PR into main. 


## What can be improved   
- Gizmos is very buggy for drag-dropping. Undo/Redo does not work for UI elements. 
- UI Scaling is poor, only looks ok when window is full size
- Potential for dropping unique pointers, and turning to manual memory management, maybe even using Sparse Sets or Archetypes, with the goal of turning this into an ECS serialized engine (hard)

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
