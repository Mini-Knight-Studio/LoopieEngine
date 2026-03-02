#pragma once

#include <string>
#include <glm/glm.hpp>

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

    class Scene;

    class AudioManager {
    public:
        static void Init();
        static void Update();
        static void Shutdown();

        static void StartSceneAudio(Scene* scene);

        static FMOD::Studio::EventInstance* CreateEventInstance(const std::string& eventPath);
        static FMOD::Sound* CreateSound(const std::string& path, bool loop = false, bool stream = false);

        static void PlaySound(FMOD::Sound* sound, FMOD::Channel** channel, bool paused = false);

        static void SetListenerAttributes(const glm::vec3& pos, const glm::vec3& forward, const glm::vec3& up);
        static FMOD_VECTOR VectorToFmod(const glm::vec3& v);
        static void SetGlobalParameter(const std::string& name, float value);

        static FMOD::Studio::System* s_StudioSystem;
        static FMOD::System* s_CoreSystem;
    };
}