#include "Audio.h"
#include "AssetManager.h"
#include "Logger.h"

Audio::Audio(const std::string& path)
{
    SetPath(path);
}

Audio::~Audio()
{
    Unload();
}

Audio::Audio(const Audio& other)
    : AssetReference(other),
      buffer(nullptr)
{
    if (!path.empty())
        Load();
}

Audio& Audio::operator=(const Audio& other)
{
    if (this == &other)
        return *this;

    Unload();

    path = other.path;

    if (!path.empty())
        Load();

    return *this;
}

Audio::Audio(Audio&& other) noexcept
    : AssetReference(std::move(other)),
      buffer(other.buffer)
{
    other.buffer = nullptr;
}

Audio& Audio::operator=(Audio&& other) noexcept
{
    if (this == &other)
        return *this;

    Unload();

    path = std::move(other.path);
    buffer = other.buffer;

    other.buffer = nullptr;

    return *this;
}

void Audio::SetPath(const std::string& newPath)
{
    if (path == newPath && IsLoaded())
        return;

    Unload();

    path = newPath;

    if (!path.empty())
        Load();
}

bool Audio::Load()
{
    if (path.empty())
    {
        buffer = nullptr;
        return false;
    }

    const std::filesystem::path resolvedPath =
        ResolvePath(path);

    AssetManager::get().LoadAudio(resolvedPath.string());

    buffer =
        AssetManager::get().GetAudio(resolvedPath.string());

    if (buffer == nullptr)
    {
        LOG_ERROR(
            "Failed to load audio asset: ",
            resolvedPath.string());

        return false;
    }

    return true;
}

void Audio::Unload()
{
    // AssetManager owns the SoundBuffer.
    // We only release our reference to it.
    buffer = nullptr;
}

bool Audio::IsLoaded() const
{
    return buffer != nullptr;
}

sf::SoundBuffer* Audio::GetBuffer() const
{
    return buffer;
}