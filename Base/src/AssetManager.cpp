#include "AssetManager.h"
#include "Logger.h"

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
		sf::Texture texture;
		if (texture.loadFromFile(path.c_str()))
		{
			textures[id] = texture;
			LOG_INFO("Texture: [" + path + "] loaded.");
		}
		else
			LOG_ERROR("Error loading image: " + path);
	}
}

void AssetManager::loadFont(const std::string& id, const std::string& path)
{
    if (fonts.count(id) == 0)
    {
        sf::Font font;
        if (font.loadFromFile(path))
        {
            fonts[id] = font;
            LOG_INFO("Font loaded: ", id.c_str());
        }
        else
            LOG_ERROR("Failed to load font: ", path.c_str());
    }
}

sf::Font* AssetManager::getFont(const std::string& id)
{
    if (fonts.count(id) > 0)
        return &fonts[id];
    LOG_WARNING("Font not found: ", id.c_str());
    return nullptr;
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
