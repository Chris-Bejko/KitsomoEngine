#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>
#include <unordered_map>
#include <filesystem>
#include "SceneManager.h"

class AssetManager
{
public:
    static AssetManager &get();

    void loadTexture(const std::string &path, bool isEditorAsset = false);
    sf::Texture *getTexture(const std::string &path, bool isEditorAsset = false);

    void loadFont(const std::string &id, const std::string &path);
    sf::Font *getFont(const std::string &id);

    void LoadAudio(const std::string &path, bool isEditorAsset = false);
    sf::SoundBuffer *GetAudio(const std::string &path);
    
    void clean();

private:
    AssetManager();
    ~AssetManager();

    AssetManager(const AssetManager &) = delete;
    AssetManager &operator=(const AssetManager &) = delete;

    std::unordered_map<std::string, sf::Texture> textures;
    std::unordered_map<std::string, sf::Font> fonts;
    std::unordered_map<std::string, sf::SoundBuffer> audioBuffers;

    std::filesystem::path ResolvePath(const std::string &path);
};