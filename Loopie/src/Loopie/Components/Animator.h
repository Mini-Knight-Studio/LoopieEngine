#pragma once
#include "Loopie/Components/Component.h"

#include "Loopie/Animations/AnimationClip.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace Loopie {

	class MeshRenderer;

	class Animator : public Component
	{
	public:
		DEFINE_TYPE(Animator)

		Animator() = default;
		~Animator();

		void Init() override;

		void SetMeshRenderer(MeshRenderer* meshRenderer);
		MeshRenderer* GetMeshRenderer() { return m_meshRenderer; }

		int GetCurrentClipIndex() const;
		const AnimationClip* GetCurrentClip() const { return m_currentClip; }
		const AnimationClip* GetClipByIndex(int index) const;

		bool IsLooping() const { return m_looping; }
		bool IsPlaying() const { return m_isPlaying; }
		void SetLooping(bool loop) { m_looping = loop; }
		float GetPlaybackSpeed() const { return m_playbackSpeed; }
		void SetPlaybackSpeed(float speed) { m_playbackSpeed = speed; }
		float GetCurrentTime() const { return m_currentTime; }

		void Play(const std::string& clipName);
		void Stop();
		void Update();

		const std::vector<glm::mat4>& GetFinalBoneMatrices() const { return m_finalBoneMatrices; }


		JsonNode Serialize(JsonNode& parent) const override;
		void Deserialize(const JsonNode& data) override;

	private:
		MeshRenderer* m_meshRenderer;
		const AnimationClip* m_currentClip = nullptr;
		int m_currentClipIndex = 0;

		float m_currentTime = 0.0f;
		float m_playbackSpeed = 1.0f;

		bool m_isPlaying = false;
		bool m_looping = true;

		std::vector<glm::mat4> m_finalBoneMatrices;
		void CalculateBoneTransform();
	};
}