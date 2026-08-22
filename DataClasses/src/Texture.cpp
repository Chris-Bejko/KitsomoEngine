#include "Texture.h"
#include "AssetManager.h"
#include "Logger.h"

Texture::Texture(const std::string &path)
{
    SetPath(path);
}

Texture::~Texture()
{
    Unload();
}

Texture::Texture(const Texture &other): AssetReference(other), texture(nullptr)
{
    if (!path.empty())
        Load();
}

Texture &Texture::operator=(const Texture &other)
{
    if (this == &other)
        return *this;

    Unload();

    path = other.path;

    if (!path.empty())
        Load();

    return *this;
}

Texture::Texture(Texture &&other) noexcept
    : AssetReference(std::move(other)),
      texture(other.texture)
{
    other.texture = nullptr;
}

Texture &Texture::operator=(Texture &&other) noexcept
{
    if (this == &other)
        return *this;

    Unload();

    path = std::move(other.path);
    texture = other.texture;

    other.texture = nullptr;

    return *this;
}

bool Texture::Load()
{
    if (path.empty())
    {
        texture = nullptr;
        return false;
    }

    const std::filesystem::path resolvedPath = ResolvePath(path);

    AssetManager::get().loadTexture(resolvedPath.string());

    texture = AssetManager::get().getTexture(resolvedPath.string());

    if (texture == nullptr)
    {
        LOG_ERROR("Failed to load texture asset: ", resolvedPath.string());

        return false;
    }

    return true;
}

void Texture::Unload()
{
    // AssetManager owns the sf::Texture.
    // We only release our reference.
    texture = nullptr;
}

bool Texture::IsLoaded() const
{
    return texture != nullptr;
}

sf::Texture *Texture::GetTexture() const
{
    return texture;
}

sf::Vector2u Texture::GetSize() const
{
    if (texture == nullptr)
        return sf::Vector2u(0, 0);

    return texture->getSize();
}