#pragma once
#include <SFML/Audio.hpp>
#include <memory>
#include <string>
#include "SerializableScript.h"
#include "Audio.h"

class AudioSource : public SerializableScript
{
    public:
        AudioSource();
        virtual ~AudioSource() = default;

        bool Init() override;
        void update(float dt) override;
        void OnFieldChanged(const std::string& fieldName) override;

        // Audio control methods
        void Play();
        void Pause();
        void Stop();
        void SetVolume(float volume);
        void SetLoop(bool loop);
        void SetPitch(float pitch);
        
        // Getters
        float GetVolume() const;
        bool IsPlaying() const;
        bool IsLooping() const;
        float GetPitch() const;
        float GetDuration() const;
        float GetPlayingOffset() const;
        
        // Setters
        void SetPlayingOffset(float offset);

        // Editor
        void DrawEditorButton() override;
        void SetAudio(const Audio& audio);
        void RefreshAudio();
    private:
        sf::Sound sound;
        Audio audioFile;
        float volume = 80.0f;
        bool isLooping = false;
        float pitch = 1.0f;
};