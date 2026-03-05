#include "AudioSource.h"
#include "Loopie/Core/AudioManager.h"
#include "Loopie/Resources/AssetRegistry.h"
#include "Loopie/Resources/ResourceManager.h"


#include <fmod_studio.hpp>
#include <fmod.hpp>
#include <cstdlib>

namespace Loopie {

    AudioSource::AudioSource() {

	}

    AudioSource::~AudioSource() {
        Stop();
    }

	void AudioSource::LoadResource() {
        if (audioClips.empty()) 
            return;
        if (currentClipIndex < 0 || currentClipIndex >= audioClips.size()) 
            return;

        auto clip = audioClips[currentClipIndex];
        if (!clip) 
            return;

        if (clip->GetAssetType() == AudioAssetType::EventAsset)
        {
            m_isEvent = true;
            clip->GetEventDescription()->createInstance(&m_eventInstance);
        }
        else
        {
            m_isEvent = false;
        }
	}

    void AudioSource::OnUpdate() {
        Transform* t = GetOwner()->GetTransform();
        if (!t) return;

        if (m_isEvent && m_eventInstance) {
            if (isSpatial) {
                FMOD_VECTOR pos = AudioManager::VectorToFmod(t->GetPosition());
                FMOD_3D_ATTRIBUTES attr = { {0} };
                attr.position = pos;
                attr.forward = AudioManager::VectorToFmod(t->Forward());
                attr.up = AudioManager::VectorToFmod(t->Up());
                m_eventInstance->set3DAttributes(&attr);
            }
        }
        else if (!m_isEvent && m_channel) {
            bool isPlaying = false;
            m_channel->isPlaying(&isPlaying);

            if (isPlaying) {
                if (isSpatial) {
                    FMOD_VECTOR pos = AudioManager::VectorToFmod(t->GetPosition());
                    FMOD_VECTOR vel = { 0, 0, 0 };
                    m_channel->set3DAttributes(&pos, &vel);
                }
            }
            else if (m_hasStarted) {
                if (isLooping) {
                    NextTrack();
                }
                else {
                    m_hasStarted = false;
                }
            }
        }
    }

    void AudioSource::NextTrack() {
        if (audioClips.empty()) return;

        if (isLooping) {
            switch (loopStrategy) {
            case AudioLoopStrategy::Sequential:
                currentClipIndex = (currentClipIndex + 1) % audioClips.size();
                break;

            case AudioLoopStrategy::Random:
                currentClipIndex = rand() % audioClips.size();
                break;

            case AudioLoopStrategy::RandomNoRepetitive:
            {
                if (audioClips.size() > 1) {
                    int newIndex;
                    do {
                        newIndex = rand() % audioClips.size();
                    } while (newIndex == currentClipIndex); 

                    currentClipIndex = newIndex;
                }
                break;
            }
            case AudioLoopStrategy::Repetitive:
            default:
                break;
            }

            Play();
        }
    }

    void AudioSource::AddClip(std::shared_ptr<AudioClip> path) {
        audioClips.push_back(path);
    }

    void AudioSource::SetCurrentClip(int index) {
        if (index >= 0 && index < audioClips.size()) {
            currentClipIndex = index;
            LoadResource();
        }
    }

    void AudioSource::Play() {
        if (audioClips.empty())
            return;

        if (!isLooping) {
            if (noLoopStrategy == AudioNoLoopStrategy::Random) {
                currentClipIndex = rand() % audioClips.size();
            }
            else if (noLoopStrategy == AudioNoLoopStrategy::First) {
                currentClipIndex = 0;
            }
        }

        if (currentClipIndex < 0 || currentClipIndex >= audioClips.size()) {
            currentClipIndex = 0;
        }

        auto clip = audioClips[currentClipIndex];
        if (!clip)
            return;

        if (m_isEvent && m_eventInstance) {
            m_eventInstance->start();
            m_hasStarted = true;

            m_eventInstance->setPitch(m_pitch);
            m_eventInstance->setVolume(m_volume);
        }
        else if (!m_isEvent && clip->GetSound()) {
            Stop();
            FMOD::Channel* newChannel = nullptr;
            AudioManager::PlaySound(clip->GetSound(), &newChannel, true);
            m_channel = newChannel;

            if (m_channel) {
                FMOD_MODE mode = isSpatial ? (FMOD_3D | FMOD_3D_LINEARROLLOFF) : FMOD_2D;
                mode |= FMOD_LOOP_NORMAL;
               

                m_channel->setMode(mode);

                if (isSpatial) {
                    m_channel->set3DMinMaxDistance(m_minDistance, m_maxDistance);

                    Transform* t = GetOwner()->GetTransform();
                    if (t) {
                        FMOD_VECTOR pos = AudioManager::VectorToFmod(t->GetPosition());
                        FMOD_VECTOR vel = { 0, 0, 0 };
                        m_channel->set3DAttributes(&pos, &vel);
                    }
                }

                m_channel->setPitch(m_pitch);
                m_channel->setVolume(m_volume);
                m_channel->setPan(m_pan);

                m_channel->setPaused(false);
                m_hasStarted = true;
            }
        }
    }

    void AudioSource::Stop() {
        m_hasStarted = false;
        if (m_isEvent && m_eventInstance) m_eventInstance->stop(FMOD_STUDIO_STOP_ALLOWFADEOUT);
        else if (!m_isEvent && m_channel) m_channel->stop();
    }

    void AudioSource::SetLoop(bool active) {
        isLooping = active;

        if (!m_isEvent && m_channel) {
            FMOD_MODE mode = isLooping ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;
            m_channel->setMode(mode);
        }
    }

    void AudioSource::SetPitch(float pitch) {
        m_pitch = pitch;

        if (m_isEvent && m_eventInstance)
            m_eventInstance->setPitch(pitch);
        else if (!m_isEvent && m_channel)
            m_channel->setPitch(pitch);
    }

    void AudioSource::SetVolume(float volume) {
        m_volume = volume;

        if (m_isEvent && m_eventInstance)
            m_eventInstance->setVolume(volume);
        else if (!m_isEvent && m_channel)
            m_channel->setVolume(volume);
    }

    void AudioSource::SetPan(float pan) {
        m_pan = pan;

        if (!m_isEvent && m_channel) {
            float left = 0.5f * (1.0f - pan);
            float right = 0.5f * (1.0f + pan);
            m_channel->setPan(pan);
        }
    }

    void AudioSource::Set3DMinMaxDistance(float minDist, float maxDist) {
        m_minDistance = minDist;
        m_maxDistance = maxDist;

        if (!m_isEvent && m_channel) {
            m_channel->set3DMinMaxDistance(minDist, maxDist);
        }
    }

    JsonNode AudioSource::Serialize(JsonNode& parent) const {
        JsonNode transformObj = parent.CreateObjectField("AudioSource");
        transformObj.CreateField("loop", isLooping);
        transformObj.CreateField("isSpatial", isSpatial);
        transformObj.CreateField("loopStrategy", static_cast<int>(loopStrategy));
        transformObj.CreateField("noLoopStrategy", static_cast<int>(noLoopStrategy));

        transformObj.CreateField("playOnAwake", playOnAwake);
        transformObj.CreateField("usePlaylist", usePlaylist);
        transformObj.CreateField("currentClipIndex", currentClipIndex);

        JsonNode clipsArray = transformObj.CreateObjectField("audioClips");
        int arrayIndex = 0;
        for (const auto& clip : audioClips) {

            clipsArray.CreateField(std::to_string(arrayIndex), clip->GetUUID().Get());
            arrayIndex++;
        }

        return transformObj;
    }

    void AudioSource::Deserialize(const JsonNode& data) {
        data.GetValue<bool>("loop", isLooping);
        data.GetValue<bool>("isSpatial", isSpatial);
        int ls = 0, nls = 0;
        auto loopStrategyResult = data.GetValue<int>("loopStrategy", ls);
        if (loopStrategyResult.Found) loopStrategy = static_cast<AudioLoopStrategy>(loopStrategyResult.Result);
        auto noLoopStrategyResult = data.GetValue<int>("noLoopStrategy", nls);
        if (noLoopStrategyResult.Found) noLoopStrategy = static_cast<AudioNoLoopStrategy>(noLoopStrategyResult.Result);

        data.GetValue<bool>("playOnAwake", playOnAwake);
        data.GetValue<bool>("usePlaylist", usePlaylist);
        data.GetValue<int>("currentClipIndex", currentClipIndex);

        audioClips.clear();

        JsonNode clipsArray = data.Child("audioClips");
        int arrayAmount = clipsArray.Size();
        for (int i = 0; i < arrayAmount; i++) {

            UUID id = UUID(clipsArray.GetValue<std::string>(std::to_string(i)).Result);
            Metadata* meta = AssetRegistry::GetMetadata(id);
            if (meta)
                AddClip(ResourceManager::GetAudioClip(*meta));
        }
    }

}