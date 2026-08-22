#pragma once
#include <string>
class Entity;

struct PlayEvent {};
struct PauseEvent {};
struct ResetEvent {};

struct SaveProjectEvent {};
struct LoadProjectEvent {};
struct OpenProjectLoadDialogEvent {};
struct ProjectLoadSuccessEvent {};
struct ProjectLoadFailedEvent {};
struct OpenProjectEvent {
    std::string projectPath;
};
struct NewProjectEvent {};

struct ReloadScriptsEvent {};


struct DeleteEntityEvent {
    Entity* entity;
};

struct OpenExportDialogEvent {};