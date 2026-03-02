#include "AudioSource.h"
#include "Loopie/Core/AudioManager.h"
#include "Loopie/Resources/AssetRegistry.h"
#include "Loopie/Resources/ResourceManager.h"


#include <fmod_studio.hpp>
#include <fmod.hpp>

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
        FMOD_VECTOR pos = AudioManager::VectorToFmod(t->GetPosition());
        FMOD_VECTOR vel = { 0, 0, 0 };

        if (m_isEvent && m_eventInstance) {
            FMOD_3D_ATTRIBUTES attr = { {0} };
            attr.position = pos;
            attr.forward = AudioManager::VectorToFmod(t->Forward());
            attr.up = AudioManager::VectorToFmod(t->Up());
            m_eventInstance->set3DAttributes(&attr);
        }
        else if (!m_isEvent && m_channel) {
            bool isPlaying = false;
            m_channel->isPlaying(&isPlaying);
            if (isPlaying) m_channel->set3DAttributes(&pos, &vel);

            if (usePlaylist && m_hasStarted)
            {

                if (!isPlaying) {
                    NextTrack();
                }
            }
        }
	}

    void AudioSource::NextTrack() {
        if (audioClips.empty()) return;

        currentClipIndex = (currentClipIndex + 1) % audioClips.size();

        SetCurrentClip(currentClipIndex);

        Play();
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
        if (audioClips.empty() || currentClipIndex < 0 || currentClipIndex >= audioClips.size())
            return;

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
            FMOD::Channel* newChannel = nullptr;
            AudioManager::PlaySound(clip->GetSound(), &newChannel, true);
            m_channel = newChannel;
            if (m_channel) {
                FMOD_MODE mode = FMOD_3D | FMOD_3D_LINEARROLLOFF;
                mode |= loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;

                m_channel->setMode(mode);
                m_channel->set3DMinMaxDistance(m_minDistance, m_maxDistance);
                m_channel->setPitch(m_pitch);
                m_channel->setVolume(m_volume);
                m_channel->setPan(m_pan);

                Transform* t = GetOwner()->GetTransform();
                if (t) {
                    FMOD_VECTOR pos = AudioManager::VectorToFmod(t->GetPosition());
                    FMOD_VECTOR vel = { 0, 0, 0 };
                    m_channel->set3DAttributes(&pos, &vel);
                }

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
        loop = active;

        if (!m_isEvent && m_channel) {
            FMOD_MODE mode = loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;
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
        transformObj.CreateField("loop", loop);
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
        data.GetValue<bool>("loop", loop);
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