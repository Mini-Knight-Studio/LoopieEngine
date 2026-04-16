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
		RendererData() {
			FinalBoneMatrices.reserve(250);
		}

		MeshRenderer* Renderer = nullptr;
		std::vector<glm::mat4> FinalBoneMatrices;
	};

	class Animator : public Component
	{
	public:
		DEFINE_TYPE(Animator)

		Animator();
		~Animator();

		void Init() override;
		void OnUpdate() override;

		void AddMeshRenderer(MeshRenderer* renderer);
		void RemoveMeshRenderer(MeshRenderer* renderer);
		void RemoveMeshRenderer(const UUID& uuid);
		RendererData* GetRendererData(const UUID& uuid);
		void ClearRenderers();
		MeshRenderer* GetFirstMeshRenderer() const;
		void SetMeshRenderer(MeshRenderer* meshRenderer); // Replaces all with single
		const std::unordered_map<UUID, RendererData>& GetRenderers() const { return m_renderers; }

		void SetTargetRenderer(MeshRenderer* renderer);
		MeshRenderer* GetTargetRenderer() const { return m_targetRenderer; }
		bool HasTargetRenderer() const { return m_targetRenderer != nullptr; }

		const AnimationClip* GetCurrentClip() const { return m_currentClip; }
		int GetCurrentClipIndex() const;
		float GetCurrentClipDuration() const { return m_currentClip ? m_currentClip->Duration : 0.0f; }

		const AnimationClip* GetNextClip() const { return m_nextClip; }
		int GetNextClipIndex() const;
		float GetNextClipDuration() const { return m_nextClip ? m_nextClip->Duration : 0.0f; }

		int GetClipIndex(const AnimationClip* clip) const;
		int GetClipIndex(const std::string& clipName) const;
		const AnimationClip* GetClipByIndex(int index) const;
		const AnimationClip* GetClipByName(const std::string clipName) const;

		float GetClipDuration(const AnimationClip* clip) const { return clip ? clip->Duration : 0.0f; }

		bool IsLooping() const { return m_looping; }
		bool IsPlaying() const { return m_isPlaying; }
		bool IsInTransition() const { return m_inTransition; }
		void SetLooping(bool loop) { m_looping = loop; }
		float GetPlaybackSpeed() const { return m_playbackSpeed; }
		void SetPlaybackSpeed(float speed) { m_playbackSpeed = speed; }
		float GetCurrentTime() const { return m_currentTime; }


		void Play(const std::string& clipName, float transitionTime = 0.25f);
		void Play();

		void Pause() { m_isPlaying = false; }
		void Resume() { m_isPlaying = true; }

		void Stop();

		void RefreshBoneCache();
		void RefreshClipCache();

		bool SelectClip(const std::string& clipName);

		bool HasAnimation() const { return m_currentClip != nullptr; }

		JsonNode Serialize(JsonNode& parent) const override;
		void Deserialize(const JsonNode& data) override;
		void Clone(const std::shared_ptr<Entity> entity, const Component& other) override;
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

		std::vector<Entity*> m_boneEntityByIndex;
		bool m_cacheDirty = true;
		bool m_clipCacheDirty = true;

		std::vector<const KeyFrame*> m_currentClipFrameCache;
		std::vector<const KeyFrame*> m_nextClipFrameCache;


		std::vector<matrix4> m_globalModelSpace;


		/// Serialization Temporal fields
		struct PendingRenderer
		{
			std::string rendererUUID;
			std::string ownerUUID;
		};
		std::vector<PendingRenderer> m_pendingRenderers;
		std::string m_pendingTargetRendererUUID;

		bool m_frameSwitch = true;

		//// Transition
		bool m_inTransition = false;
		const AnimationClip* m_nextClip = nullptr;
		int m_nextClipIndex = -1;
		float m_transitionDuration=0;
		float m_transitionTime=0;
		float m_nextTime = 0;
	};
}