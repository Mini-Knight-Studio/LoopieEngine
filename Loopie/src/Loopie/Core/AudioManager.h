#pragma once
#include <fmod_studio.hpp>
#include <fmod.hpp>
#include <string>
#include <glm/glm.hpp>

namespace Loopie { class Scene; }

namespace Loopie {

    class AudioManager {
    public:
        static void Init();
        static void Update();
        static void Shutdown();


        static void UpdateSceneAudio(Scene* scene);
        static void StartSceneAudio(Scene* scene);

        static FMOD::Studio::EventInstance* CreateEventInstance(const std::string& eventPath);
        static FMOD::Sound* CreateSound(const std::string& path, bool loop = false);

        static void PlaySound(FMOD::Sound* sound, FMOD::Channel** channel, bool paused = false);

        static void SetListenerAttributes(const glm::vec3& pos, const glm::vec3& forward, const glm::vec3& up);
        static FMOD_VECTOR VectorToFmod(const glm::vec3& v);
        static void SetGlobalParameter(const std::string& name, float value);

        static void SetTunnelZone(const glm::vec3& position, const glm::vec3& size);

        static bool IsInTunnel(const glm::vec3& pos);

        static FMOD::Studio::System* s_studioSystem;
        static FMOD::System* s_coreSystem;

        static glm::vec3 s_tunnelCenter;
        static glm::vec3 s_tunnelSize;
    };
}