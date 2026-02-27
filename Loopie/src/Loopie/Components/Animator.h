#pragma once
#include "Loopie/Components/Component.h"

#include "Loopie/Animations/AnimationClip.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace Loopie {

	class MeshRenderer;

	struct RendererData
	{
		MeshRenderer * Renderer = nullptr;
		std::vector<glm::mat4> FinalBoneMatrices;
	};

	class Animator : public Component
	{
	public:
		DEFINE_TYPE(Animator)

		Animator() = default;
		~Animator();

		void Init() override;

		void AddMeshRenderer(MeshRenderer* renderer);
		void RemoveMeshRenderer(MeshRenderer* renderer);
		void RemoveMeshRenderer(const UUID& uuid);
		RendererData GetRendererData(const UUID& uuid);
		void ClearRenderers();
		MeshRenderer* GetFirstMeshRenderer() const;
		void SetMeshRenderer(MeshRenderer* meshRenderer); // Replaces all with single
		const std::unordered_map<UUID, RendererData>& GetRenderers() const { return m_renderers; }

		void SetTargetRenderer(MeshRenderer* renderer);
		MeshRenderer* GetTargetRenderer() const { return m_targetRenderer; }
		bool HasTargetRenderer() const { return m_targetRenderer != nullptr; }

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
		void Play();

		void Pause() { m_isPlaying = false; }
		void Resume() { m_isPlaying = true; }

		void Stop();

		void Update();


		bool SelectClip(const std::string& clipName);

		bool HasAnimation() const { return m_currentClip != nullptr; }

		JsonNode Serialize(JsonNode& parent) const override;
		void Deserialize(const JsonNode& data) override;
		void OnSceneDeserialized() override;
	private:
		void CalculateBoneTransform();
	private:
		std::unordered_map<UUID, RendererData> m_renderers;
		MeshRenderer* m_targetRenderer = nullptr;

		const AnimationClip* m_currentClip = nullptr;
		int m_currentClipIndex = 0;

		float m_currentTime = 0.0f;
		float m_playbackSpeed = 1.0f;

		bool m_isPlaying = false;
		bool m_looping = true;



		/// Serialization Temporal fields
		struct PendingRenderer
		{
			std::string rendererUUID;
			std::string ownerUUID;
		};
		std::vector<PendingRenderer> m_pendingRenderers;
		std::string m_pendingTargetRendererUUID;

		bool m_frameSwitch = true;
	};
}