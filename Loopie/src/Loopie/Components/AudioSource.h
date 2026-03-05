#pragma once
#include "Loopie/Components/Component.h"
#include "Loopie/Components/Transform.h"


#include "Loopie/Resources/Types/AudioClip.h"


#include <string>
#include <vector>


namespace FMOD {
    class Sound;
    class System;
    class Channel;
    namespace Studio {
        class EventInstance;
        class System;
    }
}

struct FMOD_VECTOR;

namespace Loopie {

    enum class AudioLoopStrategy {
        Repetitive,
        Sequential,
        Random,
        RandomNoRepetitive
    };

    enum class AudioNoLoopStrategy {
        First,
        Random
    };

    class AudioSource : public Component {
    public:
        DEFINE_TYPE(AudioSource)

        AudioSource();
        ~AudioSource();


        void Init() override {};

        void OnUpdate() override;

        void NextTrack();
        void AddClip(std::shared_ptr<AudioClip> path);

        void SetCurrentClip(int index);
        void Play();
        void Stop();
        void LoadResource();

        void SetLoop(bool active);
        void SetPitch(float pitch);
        void SetVolume(float volume);
        void SetPan(float pan);
        void Set3DMinMaxDistance(float minDist, float maxDist);
		void SetIfPlayOnAwake(bool playOnAwake) { playOnAwake = playOnAwake; }

        float GetPitch() const { return m_pitch; }
        float GetVolume() const { return m_volume; }
        float GetPan() const { return m_pan; }
        bool IsLooping() const { return isLooping; }
		bool GetIfPlayOnAwake() const { return playOnAwake; }
        void Get3DMinMaxDistance(float& minDist, float& maxDist) const { minDist = m_minDistance; maxDist = m_maxDistance; }

        JsonNode Serialize(JsonNode& parent) const override;

        void Deserialize(const JsonNode& data) override;

    private:

    public:

        std::vector<std::shared_ptr<AudioClip>> audioClips;
        int currentClipIndex = 0;

        bool isSpatial = true;
        bool isLooping = false;

        AudioLoopStrategy loopStrategy = AudioLoopStrategy::Repetitive;
        AudioNoLoopStrategy noLoopStrategy = AudioNoLoopStrategy::First;

    private:

        friend class AudioManager;
        FMOD::Studio::EventInstance* m_eventInstance = nullptr;

        FMOD::Channel* m_channel = nullptr;
        bool m_isEvent = false;
        bool m_hasStarted = false;

        // Audio configuration
        float m_pitch = 1.0f;  
        float m_volume = 1.0f;   
        float m_pan = 0.0f;
        float m_minDistance = 2.0f;
        float m_maxDistance = 25.0f;

        bool playOnAwake = true;
        bool usePlaylist = false;
    };
}