#include "AudioManager.h"
#include "Loopie/Core/Log.h"
#include <fmod_errors.h>
#include <filesystem>

#include "Loopie/Scene/Scene.h"
#include "Loopie/Scene/Entity.h"
#include "Loopie/Components/AudioSource.h"
#include "Loopie/Components/AudioListener.h" 
#include "Loopie/Components/Transform.h"

#include <fmod_studio.hpp>
#include <fmod.hpp>


namespace Loopie {

    FMOD::Studio::System* AudioManager::s_StudioSystem = nullptr;
    FMOD::System* AudioManager::s_CoreSystem = nullptr;

    void AudioManager::Init() {
        FMOD_RESULT result = FMOD::Studio::System::create(&s_StudioSystem);
        if (result != FMOD_OK) {
            Log::Critical("FMOD Create Error: {0}", FMOD_ErrorString(result));
            return;
        }

        s_StudioSystem->getCoreSystem(&s_CoreSystem);

        result = s_StudioSystem->initialize(512, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, nullptr);
        if (result != FMOD_OK) {
            Log::Critical("FMOD Init Error: {0}", FMOD_ErrorString(result));
            return;
        }

        Log::Info("Audio Manager (FMOD) Initialized.");

        SetListenerAttributes({ 0,0,0 }, { 0,0,1 }, { 0,1,0 });
    }

    void AudioManager::Update() {
        if (s_StudioSystem) s_StudioSystem->update();
    }

    void AudioManager::Shutdown() {
        if (s_StudioSystem) {
            s_StudioSystem->unloadAll();
            s_StudioSystem->release();
        }
    }

    void AudioManager::StartSceneAudio(Scene* scene)
    {
        SetListenerAttributes({ 0,0,0 }, { 0,0,1 }, { 0,1,0 });

        if (!scene) return;

        auto& allEntities = scene->GetAllEntities();

        for (auto& [uuid, entity] : allEntities)
        {
            if (entity->GetIsActive())
            {
                auto source = entity->GetComponent<AudioSource>();
                if (source && source->GetIfPlayOnAwake()) {
                    if (!source->m_audioClips.empty()) {
                        source->SetCurrentClip(source->m_currentClipIndex);
                    }
                    source->Play();
                }
            }
        }
    }

    FMOD::Studio::EventInstance* AudioManager::CreateEventInstance(const std::string& eventPath) {
        FMOD::Studio::EventDescription* eventDesc = nullptr;
        FMOD_RESULT result = s_StudioSystem->getEvent(eventPath.c_str(), &eventDesc);
        if (result != FMOD_OK) {
            Log::Error("FMOD Event not found: {0}", eventPath);
            return nullptr;
        }
        FMOD::Studio::EventInstance* instance = nullptr;
        eventDesc->createInstance(&instance);
        return instance;
    }

    FMOD::Sound* AudioManager::CreateSound(const std::string& path, bool loop, bool stream) {
        FMOD::Sound* sound = nullptr;

        FMOD_MODE mode = FMOD_3D | FMOD_3D_LINEARROLLOFF | FMOD_DEFAULT;

        mode |= stream ? FMOD_CREATESTREAM : FMOD_CREATESAMPLE;
        mode |= loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;

        FMOD_RESULT result = s_CoreSystem->createSound(path.c_str(), mode, nullptr, &sound);
        if (result != FMOD_OK) {
            Log::Error("Failed to load sound: {0}", path);
            return nullptr;
        }
        return sound;
    }

    void AudioManager::PlaySound(FMOD::Sound* sound, FMOD::Channel** outChannel, bool paused) {
        if (!s_CoreSystem || !sound)
            return;

        FMOD::Channel* channel = nullptr;
        FMOD_RESULT result = s_CoreSystem->playSound(sound, nullptr, paused, &channel);

        if (result != FMOD_OK) {
            Log::Error("FMOD failed to play sound: {0}", FMOD_ErrorString(result));
            *outChannel = nullptr;
            return;
        }

        *outChannel = channel;
    }

    void AudioManager::SetListenerAttributes(const glm::vec3& pos, const glm::vec3& forward, const glm::vec3& up) {
        FMOD_3D_ATTRIBUTES attributes = { {0} };
        attributes.position = VectorToFmod(pos);
        attributes.forward = VectorToFmod(forward);
        attributes.up = VectorToFmod(up);
        if (s_StudioSystem) s_StudioSystem->setListenerAttributes(0, &attributes);
    }

    void AudioManager::SetGlobalParameter(const std::string& name, float value) {
        if (s_StudioSystem) s_StudioSystem->setParameterByName(name.c_str(), value);
    }

    FMOD_VECTOR AudioManager::VectorToFmod(const glm::vec3& v) {
        return { v.x, v.y, v.z };
    }
}