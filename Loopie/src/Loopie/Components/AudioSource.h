#pragma once
#include "Loopie/Components/Component.h"
#include "Loopie/Components/Transform.h"
#include "Loopie/Core/AudioManager.h"
#include <string>
#include <vector>

namespace Loopie {

    class AudioSource : public Component {
    public:
        DEFINE_TYPE(AudioSource)

            std::vector<std::string> audioClips;
        int currentClipIndex = 0;

        bool loop = false;
        bool playOnAwake = true;
        bool usePlaylist = false;

    private:
        friend class AudioManager;

        FMOD::Studio::EventInstance* m_eventInstance = nullptr;
        FMOD::Sound* m_sound = nullptr;
        FMOD::Channel* m_channel = nullptr;
        bool m_isEvent = false;
        bool m_hasStarted = false;

        void LoadResource() {
            if (m_eventInstance) { m_eventInstance->stop(FMOD_STUDIO_STOP_IMMEDIATE); m_eventInstance->release(); m_eventInstance = nullptr; }
            if (m_sound) { m_sound->release(); m_sound = nullptr; }
            m_channel = nullptr;

            if (audioClips.empty() || currentClipIndex < 0 || currentClipIndex >= audioClips.size()) return;

            std::string path = audioClips[currentClipIndex];
            if (path.empty()) return;

            if (path.find("event:/") == 0 || path.find("snapshot:/") == 0) {
                m_isEvent = true;
                m_eventInstance = AudioManager::CreateEventInstance(path);
            }
            else {
                m_isEvent = false;
                m_sound = AudioManager::CreateSound(path, loop);
            }
        }

    public:
        void Init() override {}

        void OnStart() override {
            LoadResource();
        }

        void OnUpdate() override {
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

        void NextTrack() {
            if (audioClips.empty()) return;

            currentClipIndex = (currentClipIndex + 1) % audioClips.size();

            SetCurrentClip(currentClipIndex);

            Play();
        }

        void SetPitch(float pitch) {
            if (m_isEvent && m_eventInstance) m_eventInstance->setPitch(pitch);
            else if (!m_isEvent && m_channel) m_channel->setPitch(pitch);
        }

        void AddClip(const std::string& path) { audioClips.push_back(path); }

        void SetCurrentClip(int index) {
            if (index >= 0 && index < audioClips.size()) {
                currentClipIndex = index;
                LoadResource();
            }
        }

        void Play() {
            if (m_isEvent && m_eventInstance) {
                m_eventInstance->start();
                m_hasStarted = true;
            }
            else if (!m_isEvent && m_sound) {
                AudioManager::PlaySound(m_sound, &m_channel, true);
                if (m_channel) {
                    m_channel->setMode(FMOD_3D | FMOD_3D_LINEARROLLOFF);
                    m_channel->set3DLevel(1.0f);
                    m_channel->set3DMinMaxDistance(2.0f, 25.0f);

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

        void Stop() {
            m_hasStarted = false;
            if (m_isEvent && m_eventInstance) m_eventInstance->stop(FMOD_STUDIO_STOP_ALLOWFADEOUT);
            else if (!m_isEvent && m_channel) m_channel->stop();
        }

        void SetLoop(bool active) {
            loop = active;

            FMOD_MODE mode = (loop && !usePlaylist) ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;

            if (!m_isEvent && m_channel) m_channel->setMode(mode);
        }

        JsonNode Serialize(JsonNode& parent) const override {
            JsonNode transformObj = parent.CreateObjectField("AudioSource");
            transformObj.CreateField("loop", loop);
            transformObj.CreateField("playOnAwake", playOnAwake);
            transformObj.CreateField("usePlaylist", usePlaylist);
            transformObj.CreateField("currentClipIndex", currentClipIndex);
            JsonNode clipsArray = transformObj.CreateObjectField("audioClips");
            int arrayIndex = 0;
            for (const auto& clip : audioClips) {

                clipsArray.CreateField(std::to_string(arrayIndex), clip);
                arrayIndex++;
            }

            return transformObj;
        }

        void Deserialize(const JsonNode& data) override {

            data.GetValue<bool>("loop", loop);
            data.GetValue<bool>("playOnAwake", playOnAwake);
            data.GetValue<bool>("usePlaylist", usePlaylist);
            data.GetValue<int>("currentClipIndex", currentClipIndex);
            audioClips.clear();

            JsonNode clipsArray = data.Child("audioClips");
            int arrayAmount = clipsArray.Size();
            for (int i = 0; i < arrayAmount; i++) {

                auto clipResult = clipsArray.GetValue<std::string>(std::to_string(i), "");
                audioClips.push_back(clipResult.Result);
            }


        }

        ~AudioSource() { Stop(); }
    };
}