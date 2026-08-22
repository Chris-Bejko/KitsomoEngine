#include "AssetManager.h"
#include "Logger.h"

#include <filesystem>

AssetManager::AssetManager()
{
}
AssetManager::~AssetManager()
{
    clean();
}

AssetManager &AssetManager::get()
{
    static AssetManager instance;
    return instance;
}

void AssetManager::loadTexture(const std::string &path, bool isEditorAsset)
{
    std::string key = std::filesystem::path(path).lexically_normal().string();
    if (!isEditorAsset)
    {
        const auto resolvedPath = ResolvePath(path);
        key = std::filesystem::weakly_canonical(resolvedPath).string();
    }

    if (textures.find(key) != textures.end())
        return;

    sf::Texture texture;

    if (!texture.loadFromFile(key))
    {
        LOG_ERROR("Failed to load texture: ", key);
        return;
    }

    textures.emplace(key, std::move(texture));

    LOG_INFO("Texture loaded: ", key, " (", textures.at(key).getSize().x, "x", textures.at(key).getSize().y, ")");
}

sf::Texture *AssetManager::getTexture(const std::string &path, bool isEditorAsset)
{
    std::string key = std::filesystem::path(path).lexically_normal().string();
    if (!isEditorAsset)
    {
        const auto resolvedPath = ResolvePath(path);
        key = std::filesystem::weakly_canonical(resolvedPath).string();
    }

    auto it = textures.find(key);

    if (it == textures.end())
    {
        LOG_WARNING("Texture not found: ", key);
        return nullptr;
    }

    return &it->second;
}

void AssetManager::loadFont(const std::string &id, const std::string &path)
{
    if (fonts.find(id) != fonts.end())
        return;

    sf::Font font;

    if (!font.loadFromFile(path))
    {
        LOG_ERROR("Failed to load font: ", path);
        return;
    }

    fonts.emplace(id, std::move(font));

    LOG_INFO("Font loaded: ", id);
}

sf::Font *AssetManager::getFont(const std::string &id)
{
    auto it = fonts.find(id);

    if (it == fonts.end())
    {
        LOG_WARNING("Font not found: ", id);
        return nullptr;
    }

    return &it->second;
}

void AssetManager::LoadAudio(const std::string &path, bool isEditorAsset)
{
    std::string key = std::filesystem::path(path).lexically_normal().string();
    if (!isEditorAsset)
    {
        const auto resolvedPath = ResolvePath(path);
        key = std::filesystem::weakly_canonical(resolvedPath).string();
    }
    if (audioBuffers.find(key) != audioBuffers.end())
        return;

    sf::SoundBuffer buffer;

    if (!buffer.loadFromFile(key))
    {
        LOG_ERROR("Failed to load audio: ", key);
        return;
    }

    audioBuffers.emplace(key, std::move(buffer));

    LOG_INFO("Audio loaded: ", key, " (", audioBuffers.at(key).getSampleCount(), " samples)");
}

sf::SoundBuffer *AssetManager::GetAudio(const std::string &path)
{
    const std::string key = std::filesystem::path(path).lexically_normal().string();

    auto it = audioBuffers.find(key);

    if (it == audioBuffers.end())
    {
        LOG_WARNING("Audio not found: ", key);
        return nullptr;
    }

    return &it->second;
}

void AssetManager::clean()
{
    textures.clear();
    audioBuffers.clear();
    fonts.clear();
}

std::filesystem::path AssetManager::ResolvePath(const std::string &path)
{
    if (std::filesystem::path(path).is_absolute())
        return path;

    return SceneManager::get().ResolveProjectPath(path);
}