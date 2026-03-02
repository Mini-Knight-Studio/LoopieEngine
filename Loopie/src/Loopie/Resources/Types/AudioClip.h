#pragma once
#include "Loopie/Resources/Resource.h"

namespace FMOD {
    class Sound;
    namespace Studio {
        class EventDescription;
    }
}

namespace Loopie {

    enum AudioAssetType /// NOT BEING USED FOR NOW (NOT WORKING)
    {
        SoundAsset,
        EventAsset
    };

    class AudioClip : public Resource {
        friend class AudioImporter;
    public:
        DEFINE_TYPE(AudioClip)

        AudioClip(const UUID& id);
        ~AudioClip();

        bool Load() override;
        void Unload();

        FMOD::Sound* GetSound() const { return m_sound; }
        FMOD::Studio::EventDescription* GetEventDescription() const { return m_eventDesc; }
        AudioAssetType GetAssetType() const { return m_assetType; }

    private:
        AudioAssetType m_assetType;

        bool m_stream = false;
        std::string m_path;

        FMOD::Sound* m_sound = nullptr;
        FMOD::Studio::EventDescription* m_eventDesc = nullptr;
    };
}