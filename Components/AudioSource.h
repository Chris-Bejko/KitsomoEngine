#pragma once
#include <SFML/Audio.hpp>
#include <memory>
#include <string>
#include "SerializableScript.h"


class AudioSource : public SerializableScript
{
    public:
        AudioSource();
        virtual ~AudioSource() = default;

        bool Init() override;
        void update(float dt) override;

        // Audio control methods
        void Play();
        void Pause();
        void Stop();
        void SetVolume(float volume);
        void SetLoop(bool loop);
        void SetPitch(float pitch);

        // Audio file management
        bool LoadAudio(const std::string& filePath);
        
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

    private:
        std::unique_ptr<sf::Sound> sound;
        std::unique_ptr<sf::SoundBuffer> soundBuffer;
        std::string currentFilePath;
        float volume = 80.0f;
        bool isLooping = false;
        float pitch = 1.0f;
};