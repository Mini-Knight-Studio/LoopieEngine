#include "AudioManager.h"
#include "Loopie/Core/Log.h"
#include <fmod_errors.h>
#include <filesystem>

#include "Loopie/Scene/Scene.h"
#include "Loopie/Scene/Entity.h"
#include "Loopie/Components/AudioSource.h"
#include "Loopie/Components/AudioListener.h" 
#include "Loopie/Components/Transform.h"


namespace Loopie {

    FMOD::Studio::System* AudioManager::s_studioSystem = nullptr;
    FMOD::System* AudioManager::s_coreSystem = nullptr;
    glm::vec3 AudioManager::s_tunnelCenter = glm::vec3(0.0f);
    glm::vec3 AudioManager::s_tunnelSize = glm::vec3(0.0f);

    void AudioManager::Init() {
        FMOD_RESULT result = FMOD::Studio::System::create(&s_studioSystem);
        if (result != FMOD_OK) {
            Log::Critical("FMOD Create Error: {0}", FMOD_ErrorString(result));
            return;
        }

        s_studioSystem->getCoreSystem(&s_coreSystem);

        result = s_studioSystem->initialize(512, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, nullptr);
        if (result != FMOD_OK) {
            Log::Critical("FMOD Init Error: {0}", FMOD_ErrorString(result));
            return;
        }

        Log::Info("Audio Manager (FMOD) Initialized.");
    }

    void AudioManager::Update() {
        if (s_studioSystem) s_studioSystem->update();
    }

    void AudioManager::Shutdown() {
        if (s_studioSystem) {
            s_studioSystem->unloadAll();
            s_studioSystem->release();
        }
    }

    void AudioManager::UpdateSceneAudio(Scene* scene)
    {
        Update();

        if (!scene) return;

        glm::vec3 listenerPos(0, 0, 0);
        bool listenerFound = false;

        auto& allEntities = scene->GetAllEntities();

        for (auto& [uuid, entity] : allEntities)
        {
            if (entity->GetIsActive())
            {
                auto listener = entity->GetComponent<AudioListener>();
                if (listener)
                {
                    Transform* t = entity->GetTransform();
                    if (t) {
                        listenerPos = t->GetPosition();
                        SetListenerAttributes(t->GetPosition(), t->Forward(), t->Up());
                        listenerFound = true;
                        break;
                    }
                }
            }
        }

        if (!listenerFound) SetListenerAttributes({ 0,0,0 }, { 0,0,1 }, { 0,1,0 });


        for (auto& [uuid, entity] : allEntities)
        {
            if (entity->GetIsActive())
            {
                auto source = entity->GetComponent<AudioSource>();
                if (source) {
                    source->OnUpdate();

                    Transform* t = entity->GetTransform();
                    if (t && AudioManager::IsInTunnel(t->GetPosition())) source->SetPitch(0.5f);
                    else source->SetPitch(1.0f);
                }
            }
        }
    }

    void AudioManager::SetTunnelZone(const glm::vec3& center, const glm::vec3& size) {
        s_tunnelCenter = center;
        s_tunnelSize = size * 0.5f;
    }

    bool AudioManager::IsInTunnel(const glm::vec3& pos) {
        bool inX = (pos.x >= s_tunnelCenter.x - s_tunnelSize.x) && (pos.x <= s_tunnelCenter.x + s_tunnelSize.x);
        bool inY = (pos.y >= s_tunnelCenter.y - s_tunnelSize.y) && (pos.y <= s_tunnelCenter.y + s_tunnelSize.y);
        bool inZ = (pos.z >= s_tunnelCenter.z - s_tunnelSize.z) && (pos.z <= s_tunnelCenter.z + s_tunnelSize.z);
        return inX && inY && inZ;
    }

    void AudioManager::StartSceneAudio(Scene* scene)
    {
        if (!scene) return;
        auto& allEntities = scene->GetAllEntities();

        for (auto& [uuid, entity] : allEntities)
        {
            if (entity->GetIsActive())
            {
                auto source = entity->GetComponent<AudioSource>();
                if (source && source->playOnAwake) {
                    if (!source->audioClips.empty()) {
                        source->SetCurrentClip(source->currentClipIndex);
                    }
                    source->Play();
                }
            }
        }
    }

    FMOD::Studio::EventInstance* AudioManager::CreateEventInstance(const std::string& eventPath) {
        FMOD::Studio::EventDescription* eventDesc = nullptr;
        FMOD_RESULT result = s_studioSystem->getEvent(eventPath.c_str(), &eventDesc);
        if (result != FMOD_OK) {
            Log::Error("FMOD Event not found: {0}", eventPath);
            return nullptr;
        }
        FMOD::Studio::EventInstance* instance = nullptr;
        eventDesc->createInstance(&instance);
        return instance;
    }

    FMOD::Sound* AudioManager::CreateSound(const std::string& path, bool loop) {
        FMOD::Sound* sound = nullptr;
        FMOD_MODE mode = FMOD_DEFAULT | FMOD_3D;
        if (loop) mode |= FMOD_LOOP_NORMAL;
        else mode |= FMOD_LOOP_OFF;

        FMOD_RESULT result = s_coreSystem->createSound(path.c_str(), mode, nullptr, &sound);
        if (result != FMOD_OK) {
            std::string fixPath = "../../" + path;
            result = s_coreSystem->createSound(fixPath.c_str(), mode, nullptr, &sound);
        }

        if (result != FMOD_OK) {
            Log::Error("Failed to load sound: {0}", path);
            return nullptr;
        }
        return sound;
    }

    void AudioManager::PlaySound(FMOD::Sound* sound, FMOD::Channel** channel, bool paused) {
        if (s_coreSystem && sound) {
            s_coreSystem->playSound(sound, nullptr, paused, channel);
        }
    }

    void AudioManager::SetListenerAttributes(const glm::vec3& pos, const glm::vec3& forward, const glm::vec3& up) {
        FMOD_3D_ATTRIBUTES attributes = { {0} };
        attributes.position = VectorToFmod(pos);
        attributes.forward = VectorToFmod(forward);
        attributes.up = VectorToFmod(up);
        if (s_studioSystem) s_studioSystem->setListenerAttributes(0, &attributes);
    }

    void AudioManager::SetGlobalParameter(const std::string& name, float value) {
        if (s_studioSystem) s_studioSystem->setParameterByName(name.c_str(), value);
    }

    FMOD_VECTOR AudioManager::VectorToFmod(const glm::vec3& v) {
        return { v.x, v.y, v.z };
    }
}