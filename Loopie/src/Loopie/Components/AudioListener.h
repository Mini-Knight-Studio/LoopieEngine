#pragma once
#include "Loopie/Components/Component.h"
#include "Loopie/Components/Transform.h"
#include "Loopie/Core/AudioManager.h"
#include "Loopie/Core/Log.h"

namespace Loopie {

    class AudioListener : public Component {
    public:
        DEFINE_TYPE(AudioListener)

            void Init() override {
            Log::Info("AudioListener INITIAL");
        }

        void OnStart() override {}

        void OnUpdate() override {
            Transform* t = GetOwner()->GetTransform();
            if (t) {
                AudioManager::SetListenerAttributes(
                    t->GetPosition(),
                    t->Forward(),
                    t->Up()
                );

            }
        }

        JsonNode Serialize(JsonNode& parent) const override {

            JsonNode transformObj = parent.CreateObjectField("AudioListener");
            return transformObj;

        }
        void Deserialize(const JsonNode& data) override {


        }
    };
}