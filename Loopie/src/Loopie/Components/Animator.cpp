#include "Animator.h"

#include "Loopie/Core/Application.h"

#include "Loopie/Components/MeshRenderer.h"
#include "Loopie/Components/Transform.h"
#include "Loopie/Core/Time.h"
#include "Loopie/Core/Log.h"
#include "Loopie/Render/Gizmo.h"

#include <algorithm>

namespace Loopie {

	Animator::~Animator() {
		ClearRenderers();
	}

	void Animator::Init()
	{
		// Nothing to initialize for now
	}

	void Animator::AddMeshRenderer(MeshRenderer* renderer)
	{
		if (!renderer)
			return;

		if (renderer->GetLinkedAnimator() && renderer->GetLinkedAnimator() != this)
			renderer->GetLinkedAnimator()->RemoveMeshRenderer(renderer);

		UUID id = renderer->GetUUID();
		if (m_renderers.find(id) != m_renderers.end())
			return;

		RendererData data;
		data.Renderer = renderer;
		m_renderers[id] = data;

		if (renderer->GetLinkedAnimator() != this)
			renderer->SetLinkedAnimator(this);

		if (!m_targetRenderer)
		{
			m_targetRenderer = renderer;
		}
	}

	void Animator::RemoveMeshRenderer(MeshRenderer* renderer)
	{
		if (!renderer) 
			return;
		RemoveMeshRenderer(renderer->GetUUID());
	}

	void Animator::RemoveMeshRenderer(const UUID& uuid)
	{
		auto it = m_renderers.find(uuid);
		if (it != m_renderers.end())
		{
			if (m_targetRenderer && m_targetRenderer->GetUUID() == uuid)
				m_targetRenderer = nullptr;

			if (it->second.Renderer && it->second.Renderer->GetLinkedAnimator() == this)
				it->second.Renderer->SetLinkedAnimator(nullptr);
			m_renderers.erase(it);
		}

		if (!m_targetRenderer && !m_renderers.empty())
		{
			m_targetRenderer = m_renderers.begin()->second.Renderer;
		}
	}

	RendererData Animator::GetRendererData(const UUID& uuid)
	{
		auto it = m_renderers.find(uuid);
		if (it != m_renderers.end())
		{
			return it->second;
		}
	}

	void Animator::ClearRenderers()
	{
		for (auto& [uuid, data] : m_renderers)
		{
			if (data.Renderer && data.Renderer->GetLinkedAnimator() == this)
				data.Renderer->SetLinkedAnimator(nullptr);
		}
		m_renderers.clear();
		m_targetRenderer = nullptr;
	}

	MeshRenderer* Animator::GetFirstMeshRenderer() const
	{
		if (m_renderers.empty()) 
			return nullptr;
		return m_renderers.begin()->second.Renderer;
	}

	void Animator::SetMeshRenderer(MeshRenderer* meshRenderer)
	{
		ClearRenderers();
		if (meshRenderer)
			AddMeshRenderer(meshRenderer);
	}

	void Animator::SetTargetRenderer(MeshRenderer* renderer)
	{
		if (renderer && m_renderers.find(renderer->GetUUID()) != m_renderers.end())
		{
			m_targetRenderer = renderer;

			if (m_targetRenderer && m_targetRenderer->GetMesh())
			{
				const auto& clips = m_targetRenderer->GetMesh()->GetData().AnimationClips;
				if (!clips.empty() && m_currentClipIndex >= static_cast<int>(clips.size()))
				{
					m_currentClipIndex = 0;
					m_currentClip = &clips[0];
				}
			}
		}
		else if (renderer == nullptr)
		{
			m_targetRenderer = nullptr;
		}
	}

	int Animator::GetCurrentClipIndex() const
	{
		return m_currentClipIndex;
	}

	const AnimationClip* Animator::GetClipByIndex(int index) const
	{
		MeshRenderer* renderer = m_targetRenderer ? m_targetRenderer : GetFirstMeshRenderer();
		if (!renderer)
			return nullptr;
		auto mesh = renderer->GetMesh();
		if (!mesh)
			return nullptr;
		const auto& clips = mesh->GetData().AnimationClips;
		if (index < 0 || index >= static_cast<int>(clips.size()))
			return nullptr;
		return &clips[index];
	}

	void Animator::Play(const std::string& clipName)
	{
		if (SelectClip(clipName))
			m_isPlaying = true;
	}

	void Animator::Play()
	{
		if (!m_targetRenderer)
		{
			m_targetRenderer = GetFirstMeshRenderer();
			if (!m_targetRenderer)
				return;
		}

		auto mesh = m_targetRenderer->GetMesh();
		if (!mesh) 
			return;

		const auto& clips = mesh->GetData().AnimationClips;
		if (clips.empty()) 
			return;

		m_currentClip = &clips[m_currentClipIndex];
		m_currentTime = 0.0f;
		m_isPlaying = true;
	}

	void Animator::Stop()
	{
		m_isPlaying = false;
		m_currentClip = nullptr;
		m_currentTime = 0.0f;
	}

	void Animator::Update()
	{
		bool frameSwitch = Application::GetInstance().GetWindow().GetFrameSwitch();

		if (m_frameSwitch == frameSwitch)
			return;

		m_frameSwitch = frameSwitch;

		if (!m_currentClip || !m_isPlaying)
			return;

		m_currentTime += m_playbackSpeed * Time::GetDeltaTime();

		float clipDuration = m_currentClip->Duration;

		if (m_looping) {
			m_currentTime = fmod(m_currentTime, clipDuration);
		}
		else {
			m_currentTime = glm::min(m_currentTime, clipDuration);
		}

		CalculateBoneTransform();
	}

	bool Animator::SelectClip(const std::string& clipName)
	{
		if (!m_targetRenderer)
		{
			m_targetRenderer = GetFirstMeshRenderer();
			if (!m_targetRenderer)
				return false;
		}

		auto mesh = m_targetRenderer->GetMesh();
		if (!mesh) 
			return false;

		int index = 0;
		for (const auto& clip : mesh->GetData().AnimationClips) {
			if (clip.Name == clipName) {
				m_currentClip = &clip;
				m_currentTime = 0.0f;
				m_currentClipIndex = index;
				return true;
			}
			++index;
		}
		return false;
	}

	JsonNode Animator::Serialize(JsonNode& parent) const
	{
		JsonNode animatorObj = parent.CreateObjectField("animator");

		if (m_targetRenderer)
		{
			animatorObj.CreateField<std::string>("targetRendererUUID", m_targetRenderer->GetUUID().Get());
		}

		JsonNode renderersArray = animatorObj.CreateArrayField("renderers");

		for (const auto& [uuid, data] : m_renderers)
		{
			if (!data.Renderer) 
				continue;

			json item;
			item["rendererUUID"] = data.Renderer->GetUUID().Get();
			item["ownerUUID"] = data.Renderer->GetOwner()->GetUUID().Get();

			renderersArray.AddArrayElement(item);
		}

		return animatorObj;
	}

	void Animator::Deserialize(const JsonNode& data)
	{
		m_pendingRenderers.clear();
		m_pendingTargetRendererUUID.clear();

		if (data.Contains("targetRendererUUID"))
		{
			m_pendingTargetRendererUUID = data.GetValue<std::string>("targetRendererUUID").Result;
		}

		if (data.Contains("renderers"))
		{
			JsonNode renderersArray = data.Child("renderers");
			unsigned int size = renderersArray.Size();

			for (unsigned int i = 0; i < size; ++i)
			{
				auto elementResult = renderersArray.GetArrayElement<json>(i);
				if (!elementResult.Found)
					continue;

				const json& item = elementResult.Result;
				if (!item.contains("rendererUUID") || !item.contains("ownerUUID"))
					continue;

				PendingRenderer pending;
				pending.rendererUUID = item["rendererUUID"].get<std::string>();
				pending.ownerUUID = item["ownerUUID"].get<std::string>();
				m_pendingRenderers.push_back(pending);
			}
		}
	}

	void Animator::OnSceneDeserialized()
	{
		for (const auto& pending : m_pendingRenderers)
		{
			if(pending.ownerUUID.empty() || pending.rendererUUID.empty())
				continue;

			std::shared_ptr<Entity> entity = Application::GetInstance().GetScene().GetEntity(UUID(pending.ownerUUID));
			if (!entity) 
				continue;

			MeshRenderer* renderer = static_cast<MeshRenderer*>(entity->GetComponent(UUID(pending.rendererUUID)));
			if (renderer)
				AddMeshRenderer(renderer);
		}

		if (!m_pendingTargetRendererUUID.empty()) {
			m_targetRenderer = GetRendererData(UUID(m_pendingTargetRendererUUID)).Renderer;
		}

		m_pendingTargetRendererUUID.clear();
		m_pendingRenderers.clear();

		if (!m_targetRenderer && !m_renderers.empty())
		{
			m_targetRenderer = m_renderers.begin()->second.Renderer;
		}
	}

	void Animator::CalculateBoneTransform()
	{
		for (auto& [uuid, data] : m_renderers)
		{
			MeshRenderer* Renderer = data.Renderer;
			if (!Renderer) continue;

			auto mesh = Renderer->GetMesh();
			if (!mesh) continue;

			const auto& skeleton = mesh->GetData().Skeleton;
			const matrix4& globalInverse = mesh->GetData().GlobalInverseTransform;
			size_t n = skeleton.size();

			data.FinalBoneMatrices.resize(n);

			std::vector<matrix4> globalTransforms(n, matrix4(1.0f));

			for (size_t i = 0; i < n; ++i)
			{
				const Bone& bone = skeleton[i];
				matrix4 localTransform;

				auto it = m_currentClip->KeyFrames.find(bone.Name);
				if (it != m_currentClip->KeyFrames.end())
				{
					const KeyFrame& keyFrame = it->second;

					vec3 pos = Vec3Key::Interpolate(keyFrame.Positions, m_currentTime);
					quaternion rot = QuaternionKey::Interpolate(keyFrame.Rotations, m_currentTime);
					vec3 scale = Vec3Key::Interpolate(keyFrame.Scales, m_currentTime);

					matrix4 T = glm::translate(matrix4(1.0f), pos);
					matrix4 R = glm::toMat4(rot);
					matrix4 S = glm::scale(matrix4(1.0f), scale);
					localTransform = T * R * S;
				}
				else
				{
					localTransform = bone.LocalBindTransform;
				}

				if (bone.ParentID >= 0)
					globalTransforms[i] = globalTransforms[bone.ParentID] * localTransform;
				else
					globalTransforms[i] = localTransform;

				data.FinalBoneMatrices[i] = globalInverse * globalTransforms[i] * bone.OffsetMatrix;
			}
		}
	}
}