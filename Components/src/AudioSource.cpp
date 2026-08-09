#include "AudioSource.h"
#include <iostream>
#include <imgui.h>
#include <algorithm>
#include "ComponentRegistry.h"

REGISTER_SERIALIZABLE_COMPONENT(AudioSource, false)

AudioSource::AudioSource()
    : sound(nullptr), soundBuffer(nullptr), currentFilePath(""),
      volume(80.0f), isLooping(false), pitch(1.0f)
{
}

bool AudioSource::Init()
{
    Field("filePath", currentFilePath);
    Field("volume", volume);
    Field("isLooping", isLooping);
    Field("pitch", pitch);
    return true;
}

void AudioSource::update(float dt)
{
    // Update can be extended for future functionality like fade-in/fade-out
}

bool AudioSource::LoadAudio(const std::string &filePath)
{
    try
    {
        // Create new sound buffer
        auto newBuffer = std::make_unique<sf::SoundBuffer>();

        // Try to load the audio file
        if (!newBuffer->loadFromFile(filePath))
        {
            std::cerr << "Failed to load audio file: " << filePath << std::endl;
            return false;
        }

        // If successful, replace the old buffer and create new sound
        soundBuffer = std::move(newBuffer);
        sound = std::make_unique<sf::Sound>(*soundBuffer);
        currentFilePath = filePath;

        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception loading audio: " << e.what() << std::endl;
        return false;
    }
}

void AudioSource::Play()
{
    if (sound)
    {
        sound->play();
    }
}

void AudioSource::Pause()
{
    if (sound)
    {
        sound->pause();
    }
}

void AudioSource::Stop()
{
    if (sound)
    {
        sound->stop();
    }
}

void AudioSource::SetVolume(float volume)
{
    if (sound)
    {
        // Clamp volume between 0 and 100
        float clampedVolume = std::max(0.0f, std::min(100.0f, volume));
        sound->setVolume(clampedVolume);
    }
}

void AudioSource::SetLoop(bool loop)
{
    if (sound)
    {
        sound->setLoop(loop);
    }
}

void AudioSource::SetPitch(float pitch)
{
    if (sound)
    {
        // Pitch should be positive
        float validPitch = std::max(0.01f, pitch);
        sound->setPitch(validPitch);
    }
}

float AudioSource::GetVolume() const
{
    if (sound)
    {
        return sound->getVolume();
    }
    return 0.0f;
}

bool AudioSource::IsPlaying() const
{
    if (sound)
    {
        return sound->getStatus() == sf::Sound::Playing;
    }
    return false;
}

bool AudioSource::IsLooping() const
{
    if (sound)
    {
        return sound->getLoop();
    }
    return false;
}

float AudioSource::GetPitch() const
{
    if (sound)
    {
        return sound->getPitch();
    }
    return 1.0f;
}

float AudioSource::GetDuration() const
{
    if (soundBuffer)
    {
        return soundBuffer->getDuration().asSeconds();
    }
    return 0.0f;
}

float AudioSource::GetPlayingOffset() const
{
    if (sound)
    {
        return sound->getPlayingOffset().asSeconds();
    }
    return 0.0f;
}

void AudioSource::SetPlayingOffset(float offset)
{
    if (sound)
    {
        sound->setPlayingOffset(sf::seconds(offset));
    }
}
    
void AudioSource::DrawEditorButton()
{
    ImGui::Spacing();
    ImGui::Separator();

    // Playback controls
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Playback Controls");

    float buttonWidth = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * 2) / 3.0f;

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.5f, 0.2f, 1.0f));
    if (ImGui::Button("Play", ImVec2(buttonWidth, 0)))
        Play();
    ImGui::PopStyleColor();

    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.5f, 0.2f, 1.0f));
    if (ImGui::Button("Pause", ImVec2(buttonWidth, 0)))
        Pause();
    ImGui::PopStyleColor();

    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.2f, 0.2f, 1.0f));
    if (ImGui::Button("Stop", ImVec2(buttonWidth, 0)))
        Stop();
    ImGui::PopStyleColor();

    // Status display
    ImGui::Spacing();
    if (IsPlaying())
        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Status: Playing");
    else if (sound && sound->getStatus() == sf::Sound::Paused)
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.2f, 1.0f), "Status: Paused");
    else
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Status: Stopped");

    // Audio info
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Audio Duration: %.2f s", GetDuration());
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Playing Offset: %.2f s", GetPlayingOffset());

    // File loaded
    if (!currentFilePath.empty())
        ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "File: %s", currentFilePath.c_str());
    else
        ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.2f, 1.0f), "No audio file loaded");
}
