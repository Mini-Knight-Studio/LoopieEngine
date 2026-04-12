#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <glm/glm.hpp>

namespace FMOD {
    class Sound;
	class System;
	class Channel;
	class ChannelGroup;
    namespace Studio {
        class EventInstance;
		class System;
		class Bus;
	}
}

struct FMOD_VECTOR;


namespace Loopie {

    class Scene;

    struct AudioBus {
        std::string name;
        std::string path;

        FMOD::ChannelGroup* group = nullptr;
        AudioBus* parent = nullptr;
        float volume = 1.0f;

        std::unordered_map<std::string, std::unique_ptr<AudioBus>> children;
    };

    class AudioManager {
    public:
        static void Init();
        static void Update();
        static void Shutdown();

		static FMOD::Studio::System* GetStudioSystem() { return s_StudioSystem; }
		static FMOD::System* GetCoreSystem() { return s_CoreSystem; }

        static void StartSceneAudio(Scene* scene);

        static FMOD::Studio::EventInstance* CreateEventInstance(const std::string& eventPath);
        static FMOD::Sound* CreateSound(const std::string& path, bool loop = false, bool stream = false);

        static bool CreateBus(const std::string& path);
        static bool RemoveBus(AudioBus* bus);
        static AudioBus* GetBus(const std::string& path);
        static void SetBusVolume(const std::string& path, float volume);
        static float GetBusVolume(const std::string& path);
        static AudioBus* GetMasterBus() { return s_MasterBus.get(); }

        static void PlaySound(FMOD::Sound* sound, FMOD::Channel** channel, bool paused = false);

        static void SetListenerAttributes(const glm::vec3& pos, const glm::vec3& forward, const glm::vec3& up);
        static FMOD_VECTOR VectorToFmod(const glm::vec3& v);
        static void SetGlobalParameter(const std::string& name, float value);

        static void SaveAudioMixer();

    private:
        static FMOD::Studio::System* s_StudioSystem;
        static FMOD::System* s_CoreSystem;

        static std::unique_ptr<AudioBus> s_MasterBus;
    };
}