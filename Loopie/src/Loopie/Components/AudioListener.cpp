#include "AudioListener.h"

#include "Loopie/Core/Application.h"
#include "Loopie/Core/Log.h"
#include "Loopie/Components/Transform.h"
#include "Loopie/Audio/AudioManager.h"

#include "Loopie/Profiler/Profiler.h"

namespace Loopie {
	void AudioListener::Init() {
		Log::Info("AudioListener INITIAL");
	}

	void AudioListener::OnUpdate() {
		LP_FUNC();

		Transform* transform = GetOwner()->GetTransform();

		if (m_rotationTarget.lock())
			AudioManager::SetListenerAttributes(transform->GetPosition(), transform->Forward(), transform->Up());
		else {
			Transform* transformRotation = m_rotationTarget.lock()->GetTransform();
			AudioManager::SetListenerAttributes(transform->GetPosition(), transformRotation->Forward(), transformRotation->Up());
		}
	}

	JsonNode AudioListener::Serialize(JsonNode& parent) const
	{
		JsonNode audioListenerObj = parent.CreateObjectField("audiolistener");

		if(m_rotationTarget.lock())
			audioListenerObj.CreateField<std::string>("rotationEntityUUID", m_rotationTarget.lock()->GetUUID().Get());
		return audioListenerObj;
	}

	void AudioListener::Deserialize(const JsonNode& data) {

		if (data.Contains("rotationEntityUUID"))
			m_rotationTargetPending = data.GetValue<std::string>("rotationEntityUUID", "").Result;

	}

	void AudioListener::OnSceneDeserialized()
	{
		if (!m_rotationTargetPending.empty())
		{
			std::shared_ptr<Entity> entity = Application::GetInstance().GetScene().GetEntity(UUID(m_rotationTargetPending));
			m_rotationTarget = entity;
		}
	}

	void AudioListener::Clone(const std::shared_ptr<Entity> entity, const Component& other)
	{
		const AudioListener& otherListener = static_cast<const AudioListener&>(other);
		m_rotationTarget = otherListener.m_rotationTarget;
	}
}