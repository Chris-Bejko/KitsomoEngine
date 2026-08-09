#pragma once

#include <map>
#include <string>
#include <iostream>
#include "SFML/Graphics.hpp"

class AssetManager
{
public:
    AssetManager();
    ~AssetManager() = default;

    sf::Texture getTexture(std::string id);
    void loadTexture(std::string id, std::string path);

    sf::Font* getFont(const std::string& id);
    void loadFont(const std::string& id, const std::string& path);

    void clean();
    inline static AssetManager& get()
    {
        static AssetManager instance;
        return instance;
    }

private:
    std::map<std::string, sf::Font> fonts;
    std::map<std::string, sf::Texture> textures;

};