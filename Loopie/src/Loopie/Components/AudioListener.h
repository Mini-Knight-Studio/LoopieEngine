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
        void OnSceneDeserialized() override;
		void Clone(const std::shared_ptr<Entity> entity, const Component& other) override;

		std::weak_ptr<Entity> GetRotationTarget() const { return m_rotationTarget; }
		void SetRotationTarget(const std::weak_ptr<Entity>& target) { m_rotationTarget = target; }
		void RemoveRotationTarget() { m_rotationTarget.reset(); }
    private:
        std::weak_ptr<Entity> m_rotationTarget;
        std::string m_rotationTargetPending;
    };
}