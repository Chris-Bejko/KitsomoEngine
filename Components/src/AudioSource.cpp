#include "AudioSource.h"
#include <iostream>
#include <imgui.h>
#include <algorithm>
#include "ComponentRegistry.h"
#include "AssetManager.h"

DECLARE_COMPONENT_RULES(AudioSource, false)
REGISTER_COMPONENT(AudioSource)
REGISTER_SERIALIZABLE_COMPONENT(AudioSource)

AudioSource::AudioSource()
    : sound(), soundBuffer(nullptr), audioFile(),
      volume(80.0f), isLooping(false), pitch(1.0f)
{
}

bool AudioSource::Init()
{
    // Field("AudioFile", audioFile);
    Field("volume", volume);
    Field("isLooping", isLooping);
    Field("pitch", pitch);
    RefreshAudio();
    sound.setVolume(volume);
    sound.setLoop(isLooping);
    sound.setPitch(pitch);
    return true;
}

void AudioSource::update(float dt)
{
    // Update can be extended for future functionality like fade-in/fade-out
}

void AudioSource::SetAudio(const Audio& audio)
{
    Stop();

    audioFile = audio;
    RefreshAudio();
}

void AudioSource::RefreshAudio()
{
    sound.stop();
    soundBuffer = nullptr;

    const std::string& path = audioFile.GetPath();

    if (path.empty())
    {
        sound.resetBuffer();
        return;
    }

    AssetManager::get().LoadAudio(path);

    soundBuffer = AssetManager::get().GetAudio(path);

    if (soundBuffer == nullptr)
    {
        sound.resetBuffer();
        LOG_ERROR("AudioSource failed to resolve audio: ", path);
        return;
    }

    sound.setBuffer(*soundBuffer);
    sound.setVolume(volume);
    sound.setLoop(isLooping);
    sound.setPitch(pitch);
}

void AudioSource::Play()
{
    sound.play();
}

void AudioSource::Pause()
{
    sound.pause();
}

void AudioSource::Stop()
{
    sound.stop();
}

void AudioSource::SetVolume(float volume)
{
    // Clamp volume between 0 and 100
    float clampedVolume = std::max(0.0f, std::min(100.0f, volume));
    sound.setVolume(clampedVolume);
}

void AudioSource::SetLoop(bool loop)
{
    sound.setLoop(loop);
}

void AudioSource::SetPitch(float pitch)
{
    // Pitch should be positive
    float validPitch = std::max(0.01f, pitch);
    sound.setPitch(validPitch);
}

float AudioSource::GetVolume() const
{
    return sound.getVolume();
}

bool AudioSource::IsPlaying() const
{
    return sound.getStatus() == sf::Sound::Playing;
}

bool AudioSource::IsLooping() const
{
    return sound.getLoop();
}

float AudioSource::GetPitch() const
{
    return sound.getPitch();
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
    return sound.getPlayingOffset().asSeconds();
}

void AudioSource::SetPlayingOffset(float offset)
{
    sound.setPlayingOffset(sf::seconds(offset));
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
    else if (sound.getStatus() == sf::Sound::Paused)
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.2f, 1.0f), "Status: Paused");
    else
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Status: Stopped");

    // Audio info
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Audio Duration: %.2f s", GetDuration());
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Playing Offset: %.2f s", GetPlayingOffset());

    // File loaded
    if (!audioFile.GetPath().empty())
        ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "File: %s", audioFile.GetName().c_str());
    else
        ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.2f, 1.0f), "No audio file loaded");
}


void AudioSource::OnFieldChanged(const std::string& fieldName)
{
    if (fieldName == "AudioFile")
    {
        RefreshAudio();
    }
}