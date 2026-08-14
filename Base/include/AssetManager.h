#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>

class AssetManager
{
public:
    static AssetManager& get();

    void loadTexture(const std::string& path);
    sf::Texture* getTexture(const std::string& path);

    void loadFont(const std::string& id, const std::string& path);
    sf::Font* getFont(const std::string& id);

    void clean();

private:
    AssetManager();
    ~AssetManager();

    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;

    std::unordered_map<std::string, sf::Texture> textures;
    std::unordered_map<std::string, sf::Font> fonts;
};