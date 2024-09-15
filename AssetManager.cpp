#include "AssetManager.h"
#include <iostream>
#include "../Engine.h"
#include "SFML/Graphics.hpp"

AssetManager* AssetManager::s_instance = nullptr;

AssetManager::AssetManager()
{
	//if (TTF_Init() != 0)
	  //  std::cerr << TTF_GetError() << std::endl;

	//if (IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) != 0)
	  //  std::cerr << TTF_GetError() << std::endl;
}

sf::Texture AssetManager::getTexture(std::string id)
{
	return textures[id];
}

void AssetManager::loadTexture(std::string id, std::string path)
{
	if (textures.count(id) <= 0)
	{
		//sf::Texture* texture.loadFromFile() = IMG_LoadTexture(Engine::get().GetRenderer(), path.c_str());
		sf::Texture texture;
		if (texture.loadFromFile(path.c_str()))
		{
			textures[id] = texture;
			std::cout << "texture: [" << path << "] loaded." << std::endl;
		}
		else
			std::cerr << "Error loading image " << std::endl;
	}
}

//TTF_Font* AssetManager::getFont(std::string id)
//{
//	if (fonts.count(id) > 0)
//	{
//		return fonts[id];
//	}
//	return nullptr;
//}

void AssetManager::loadFont(std::string id, std::string path, int fontSize)
{
	/* TTF_Font *newFont = TTF_OpenFont(path.c_str(), fontSize);
	 if (newFont != nullptr)
	 {
		 fonts.emplace(id, newFont);
		 std::cout << "font: [" << path << "] loaded." << std::endl;
	 }
	 else
		 std::cerr << TTF_GetError() << std::endl;
		 */
}
void AssetManager::clean()
{
	/*for (auto it = textures.begin(); it != textures.end(); it++)
	{
		SDL_DestroyTexture(it->second);
		textures.erase(it);
	}

	textures.clear();

	for (auto it = fonts.begin(); it != fonts.end(); it++)
	{
		TTF_CloseFont(it->second);
		fonts.erase(it);
	}

	fonts.clear();
	std::cout << "Assets Cleared." << std::endl;*/
}
