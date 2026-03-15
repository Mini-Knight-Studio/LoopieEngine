#include "AudioListener.h"

#include "Loopie/Core/Log.h"
#include "Loopie/Components/Transform.h"
#include "Loopie/Audio/AudioManager.h"

namespace Loopie {
	void AudioListener::Init() {
		Log::Info("AudioListener INITIAL");
	}

	void AudioListener::OnUpdate() {
		Transform* transform = GetOwner()->GetTransform();
		if (transform) {
			AudioManager::SetListenerAttributes(transform->GetPosition(), transform->Forward(), transform->Up());
		}
	}

	JsonNode AudioListener::Serialize(JsonNode& parent) const
	{
		JsonNode transformObj = parent.CreateObjectField("audiolistener");
		return transformObj;
	}

	void AudioListener::Deserialize(const JsonNode& data) {
		if (!data.Contains("audiolistener")) return;
		JsonNode transformObj = data.Child("audiolistener");
	}
}