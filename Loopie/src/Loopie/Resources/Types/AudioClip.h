#pragma once
#include "Loopie/Resources/Resource.h"

namespace Loopie {
    class AudioClip : public Resource {
    public:
        std::string eventPath;

        AudioClip(const std::string& path) : Resource(ResourceType::Audio, path) {
            eventPath = path;
        }
    };
}