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

    //TTF_Font* getFont(std::string id);
    void loadFont(std::string id, std::string path, int fontSize);

    void clean();
    inline static AssetManager& get()
    {
        if (s_instance == nullptr)
        {
            s_instance = new AssetManager();
        }

        return *s_instance;
    }

private:
    static AssetManager* s_instance;
    //std::map<std::string, TTF_Font*> fonts;
    std::map<std::string, sf::Texture> textures;

};