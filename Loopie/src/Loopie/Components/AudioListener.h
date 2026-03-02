#pragma once
#include "Loopie/Components/Component.h"

namespace Loopie {

    class AudioListener : public Component {
    public:
        DEFINE_TYPE(AudioListener)

        void Init() override;

        void OnUpdate() override;

        JsonNode Serialize(JsonNode& parent) const override;
        void Deserialize(const JsonNode& data) override;
    };
}