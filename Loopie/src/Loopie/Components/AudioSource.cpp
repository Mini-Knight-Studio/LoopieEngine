#include "AudioSource.h"
#include "Loopie/Audio/AudioManager.h"
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
        m_audioClips.clear();
    }

	void AudioSource::LoadResource() {
        if (m_audioClips.empty()) 
            return;
        if (m_currentClipIndex < 0 || m_currentClipIndex >= m_audioClips.size())
            return;

        auto clip = m_audioClips[m_currentClipIndex];
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
            if (m_isSpatial) {
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
                if (m_isSpatial) {
                    FMOD_VECTOR pos = AudioManager::VectorToFmod(t->GetPosition());
                    FMOD_VECTOR vel = { 0, 0, 0 };
                    m_channel->set3DAttributes(&pos, &vel);
                }
            }
            else if (m_hasStarted) {
                if (m_isLooping) {
                    NextTrack();
                }
                else {
                    m_hasStarted = false;
                }
            }
        }
    }

    void AudioSource::NextTrack() {
        if (m_audioClips.empty()) return;

        if (m_isLooping) {
            switch (m_loopStrategy) {
            case AudioLoopStrategy::Sequential:
                m_currentClipIndex = (m_currentClipIndex + 1) % m_audioClips.size();
                break;

            case AudioLoopStrategy::Random:
                m_currentClipIndex = rand() % m_audioClips.size();
                break;

            case AudioLoopStrategy::RandomNoRepetitive:
            {
                if (m_audioClips.size() > 1) {
                    int newIndex;
                    do {
                        newIndex = rand() % m_audioClips.size();
                    } while (newIndex == m_currentClipIndex);

                    m_currentClipIndex = newIndex;
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

    void AudioSource::AddClip(std::shared_ptr<AudioClip> clip) {
        if (clip) {
            m_audioClips.push_back(clip);
        }
    }

    void AudioSource::RemoveClip(std::shared_ptr<AudioClip> clip)
    {
		auto it = std::find(m_audioClips.begin(), m_audioClips.end(), clip);
        if (it != m_audioClips.end()) {
            m_audioClips.erase(it);
        }
    }

    void AudioSource::RemoveClip(int index)
    {
        if (index >= 0 && index < m_audioClips.size()) {
            auto it = m_audioClips.begin() + index;
            m_audioClips.erase(it);
        }
    }

    void AudioSource::SetCurrentClip(int index) {
        if (index >= 0 && index < m_audioClips.size()) {
            m_currentClipIndex = index;
            LoadResource();
        }
    }

    void AudioSource::Play() {
        if (m_audioClips.empty())
            return;

        if (!m_isLooping) {
            if (m_noLoopStrategy == AudioNoLoopStrategy::Random) {
                m_currentClipIndex = rand() % m_audioClips.size();
            }
            else if (m_noLoopStrategy == AudioNoLoopStrategy::First) {
                m_currentClipIndex = 0;
            }
        }

        if (m_currentClipIndex < 0 || m_currentClipIndex >= m_audioClips.size()) {
            m_currentClipIndex = 0;
        }

        auto clip = m_audioClips[m_currentClipIndex];
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
                FMOD_MODE mode = m_isSpatial ? (FMOD_3D | FMOD_3D_LINEARROLLOFF) : FMOD_2D;
                mode |= FMOD_LOOP_OFF;
               

                m_channel->setMode(mode);

                if (m_isSpatial) {
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
        m_isLooping = active;
        UpdateChannelMode();
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

    void AudioSource::UpdateChannelMode() {
        if (!m_isEvent && m_channel) {

            FMOD_MODE mode = m_isSpatial ? (FMOD_3D | FMOD_3D_LINEARROLLOFF) : FMOD_2D;

            if (m_isLooping && m_loopStrategy == AudioLoopStrategy::Repetitive) {
                mode |= FMOD_LOOP_NORMAL;
            }
            else {
                mode |= FMOD_LOOP_OFF;
            }

            m_channel->setMode(mode);
        }
    }


    void AudioSource::SetSpatial(bool active) {
        m_isSpatial = active;
        UpdateChannelMode();
    }

    void AudioSource::SetLoopStrategy(AudioLoopStrategy strategy) {
        m_loopStrategy = strategy;
        UpdateChannelMode();
    }

    void AudioSource::SetNoLoopStrategy(AudioNoLoopStrategy strategy) {
        m_noLoopStrategy = strategy;
        
    }

    JsonNode AudioSource::Serialize(JsonNode& parent) const {
        JsonNode transformObj = parent.CreateObjectField("audiosource");
        transformObj.CreateField("loop", m_isLooping);
        transformObj.CreateField("isSpatial", m_isSpatial);
        transformObj.CreateField("loopStrategy", static_cast<int>(m_loopStrategy));
        transformObj.CreateField("noLoopStrategy", static_cast<int>(m_noLoopStrategy));

        transformObj.CreateField("playOnAwake", m_playOnAwake);
        transformObj.CreateField("pitch", m_pitch);
        transformObj.CreateField("volume", m_volume);
        transformObj.CreateField("pan", m_pan);
        transformObj.CreateField("minDistance", m_minDistance);
        transformObj.CreateField("maxDistance", m_maxDistance);
        transformObj.CreateField("currentClipIndex", m_currentClipIndex);

        JsonNode clipsArray = transformObj.CreateObjectField("audioClips");
        int arrayIndex = 0;
        for (const auto& clip : m_audioClips) {

            clipsArray.CreateField(std::to_string(arrayIndex), clip->GetUUID().Get());
            arrayIndex++;
        }

        return transformObj;
    }

    void AudioSource::Deserialize(const JsonNode& data) {
        m_isLooping = data.GetValue<bool>("loop", false).Result;
        m_isSpatial = data.GetValue<bool>("isSpatial", true).Result;
        int ls = 0, nls = 0;
        auto loopStrategyResult = data.GetValue<int>("loopStrategy", ls);
        if (loopStrategyResult.Found) 
            m_loopStrategy = static_cast<AudioLoopStrategy>(loopStrategyResult.Result);
        auto noLoopStrategyResult = data.GetValue<int>("noLoopStrategy", nls);
        if (noLoopStrategyResult.Found) 
            m_noLoopStrategy = static_cast<AudioNoLoopStrategy>(noLoopStrategyResult.Result);

        m_playOnAwake = data.GetValue<bool>("playOnAwake", true).Result;
        m_pitch = data.GetValue<float>("pitch", 1).Result;
        m_volume = data.GetValue<float>("volume", 1).Result;
        m_pan = data.GetValue<float>("pan", 0).Result;
        m_minDistance = data.GetValue<float>("minDistance", 2).Result;
        m_maxDistance = data.GetValue<float>("maxDistance", 100).Result;
        m_currentClipIndex = data.GetValue<int>("currentClipIndex", 0).Result;

        m_audioClips.clear();

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