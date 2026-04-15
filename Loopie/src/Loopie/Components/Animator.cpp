#include "Animator.h"

#include "Loopie/Core/Application.h"

#include "Loopie/Components/MeshRenderer.h"
#include "Loopie/Components/Transform.h"
#include "Loopie/Core/Time.h"
#include "Loopie/Core/Log.h"
#include "Loopie/Render/Gizmo.h"

#include "Loopie/Profiler/Profiler.h"

#include <algorithm>

namespace Loopie {
	Animator::Animator()
	{
		m_renderers.reserve(10);
	}

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
		data.FinalBoneMatrices.resize(renderer->GetMesh()->GetData().Skeleton.size(), matrix4(1));
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

		return RendererData();
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

	int Animator::GetNextClipIndex() const
	{
		if (!m_inTransition)
			return -1;
		return m_nextClipIndex;
	}	

	int Animator::GetClipIndex(const AnimationClip* clip) const
	{
		MeshRenderer* renderer = m_targetRenderer ? m_targetRenderer : GetFirstMeshRenderer();
		if (!renderer)
			return -1;
		auto mesh = renderer->GetMesh();
		if (!mesh)
			return -1;
		const auto& clips = mesh->GetData().AnimationClips;
		int index = 0;
		for (const auto& clipObj : clips) {
			if(clip == &clipObj)
				return index;
			++index;
		}
		return -1;
	}

	int Animator::GetClipIndex(const std::string& clipName) const
	{
		MeshRenderer* renderer = m_targetRenderer ? m_targetRenderer : GetFirstMeshRenderer();
		if (!renderer)
			return -1;
		auto mesh = renderer->GetMesh();
		if (!mesh)
			return -1;
		const auto& clips = mesh->GetData().AnimationClips;
		int index = 0;
		for (const auto& clipObj : clips) {
			if (clipName == clipObj.Name)
				return index;
			++index;
		}
		return -1;
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

	const AnimationClip* Animator::GetClipByName(const std::string clipName) const
	{
		MeshRenderer* renderer = m_targetRenderer ? m_targetRenderer : GetFirstMeshRenderer();
		if (!renderer)
			return nullptr;
		auto mesh = renderer->GetMesh();
		if (!mesh)
			return nullptr;
		const auto& clips = mesh->GetData().AnimationClips;
		for (const auto& clip : clips) {
			if (clip.Name == clipName) {
				return &clip;
			}
		}
		return nullptr;
	}

	void Animator::Play(const std::string& clipName, float transitionTime)
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

		int index = 0;
		for (const auto& clip : mesh->GetData().AnimationClips) {
			if (clip.Name == clipName) {
				if (!m_currentClip || transitionTime <= 0) {
					m_currentClip = &clip;
					m_currentTime = 0.0f;
					m_currentClipIndex = index;
				}
				else {
					m_nextClip = &clip;
					m_nextClipIndex = index;

					m_transitionDuration = std::max(transitionTime, 0.0001f);
					m_transitionTime = 0;
					m_nextTime = 0;
					m_inTransition = true;
				}
				
				m_isPlaying = true;
			}
			++index;
		}
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

	void Animator::Stop()
	{
		m_isPlaying = false;
		m_currentClip = nullptr;
		m_currentTime = 0.0f;
	}

	void Animator::RefreshBoneCache()
	{
		MeshRenderer* renderer = m_targetRenderer ? m_targetRenderer : GetFirstMeshRenderer();
		if (!renderer) return;

		auto mesh = renderer->GetMesh();
		if (!mesh) return;

		const auto& skeleton = mesh->GetData().Skeleton;

		m_boneEntityByIndex.clear();
		m_boneEntityByIndex.resize(skeleton.size(), nullptr);

		m_globalModelSpace.resize(skeleton.size(), matrix4(1));

		std::unordered_map<std::string, Entity*> nameToEntity;
		nameToEntity.reserve(150);

		std::function<void(Entity*)> traverse = [&](Entity* entity)
			{
				nameToEntity[entity->GetName()] = entity;

				for (auto& child : entity->GetChildren())
					traverse(child.get());
			};

		traverse(GetOwner().get());

		for (size_t i = 0; i < skeleton.size(); ++i)
		{
			auto it = nameToEntity.find(skeleton[i].Name);
			if (it != nameToEntity.end())
				m_boneEntityByIndex[i] = it->second;
		}

		m_cacheDirty = false;
	}

	void Animator::OnUpdate()
	{

		LP_FUNC();

		bool frameSwitch = Application::GetInstance().GetWindow().GetFrameSwitch();

		if (m_frameSwitch == frameSwitch)
			return;

		float dt = (float)Time::GetDeltaTime();

		m_frameSwitch = frameSwitch;

		if (m_inTransition) {
			m_nextTime += m_playbackSpeed * dt;
			if (m_transitionTime >= m_transitionDuration) {
				m_currentClip = m_nextClip;
				m_currentClipIndex = m_nextClipIndex;
				m_inTransition = false;
			}
			m_transitionTime += m_playbackSpeed * dt;
		}

		if (m_currentClip && m_isPlaying)
		{
			m_currentTime += m_playbackSpeed * dt;

			float clipDuration = m_currentClip->Duration;

			if (m_looping) {
				m_currentTime = fmod(m_currentTime, clipDuration);
			}
			else {
				m_currentTime = glm::min(m_currentTime, clipDuration);
			}
		}

		

		CalculateBoneTransform();
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

	void Animator::Clone(const std::shared_ptr<Entity> entity, const Component& other)
	{
		const Animator& otherAnimator = static_cast<const Animator&>(other);
		m_targetRenderer = nullptr;
		m_currentClip = nullptr;
		m_nextClip = nullptr;
		m_currentTime = 0.0f;
		m_nextTime = 0.0f;
		m_currentClipIndex = 0;
		m_nextClipIndex = 0;
		m_inTransition = false;
		m_isPlaying = otherAnimator.m_isPlaying;
		m_looping = otherAnimator.m_looping;
		m_playbackSpeed = otherAnimator.m_playbackSpeed;

		std::unordered_map<UUID, RendererData> rendererUUIDMap = otherAnimator.m_renderers;

		for (const auto& [uuid, data] : rendererUUIDMap)
		{
			std::shared_ptr<Entity> renderEntity = data.Renderer->GetOwner();
			std::shared_ptr<Entity> similEntity = nullptr;
			if (renderEntity == entity)
				similEntity = GetOwner();
			else
				similEntity = GetOwner()->GetChild(renderEntity->GetName());

			if(similEntity == nullptr)
				continue;

			std::vector<MeshRenderer*> renderers = renderEntity->GetComponents<MeshRenderer>();

			for (size_t i = 0; i < renderers.size(); i++)
			{
				if (renderers[i] == data.Renderer)
				{
					AddMeshRenderer(similEntity->GetComponents<MeshRenderer>()[i]);
					continue;
				}
			}
		}
		if (otherAnimator.m_targetRenderer)
		{

			std::shared_ptr<Entity> renderEntity = otherAnimator.m_targetRenderer->GetOwner();

			std::shared_ptr<Entity> similEntity = nullptr;
			if (renderEntity == entity)
				similEntity = GetOwner();
			else
				similEntity = GetOwner()->GetChild(renderEntity->GetName());

			if (similEntity != nullptr)
			{
				std::vector<MeshRenderer*> renderers = renderEntity->GetComponents<MeshRenderer>();

				for (size_t i = 0; i < renderers.size(); i++)
				{
					if (renderers[i] == otherAnimator.m_targetRenderer)
					{
						SetTargetRenderer(similEntity->GetComponents<MeshRenderer>()[i]);
						break;
					}
				}
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

	void Animator::CalculateBoneTransform() {
		if (m_cacheDirty) {
			RefreshBoneCache();
		}

		MeshRenderer* renderer = m_targetRenderer ? m_targetRenderer : GetFirstMeshRenderer();
		if (!renderer)
			return;
		auto mesh = renderer->GetMesh();
		if (!mesh)
			return;
		auto skeleton = mesh->GetData().Skeleton;
		const matrix4& globalInverse = mesh->GetData().GlobalInverseTransform;

		size_t n = skeleton.size();
		const auto& boneEntities = m_boneEntityByIndex;

		if (m_currentClip && m_isPlaying) {
			for (size_t i = 0; i < n; ++i) {
				Entity* entity = boneEntities[i];
				if (!entity) 
					continue;
				auto* transform = entity->GetTransform();

				const Bone& bone = skeleton[i];
				auto keyIt = m_currentClip->KeyFrames.find(bone.Name);
				if (keyIt != m_currentClip->KeyFrames.end()) {
					const KeyFrame& keyFrame = keyIt->second;

					vec3 pos;
					quaternion rot;
					vec3 scale;

					if (m_inTransition) {

						float t = std::clamp(m_transitionTime / m_transitionDuration, 0.f, 1.f);
						auto nextKeyFrame = m_nextClip->KeyFrames.at(bone.Name);

						vec3 posA = Vec3Key::Interpolate(keyFrame.Positions, m_currentTime);
						vec3 posB = Vec3Key::Interpolate(nextKeyFrame.Positions, m_nextTime);

						quaternion rotA = QuaternionKey::Interpolate(keyFrame.Rotations, m_currentTime);
						quaternion rotB = QuaternionKey::Interpolate(nextKeyFrame.Rotations, m_nextTime);

						vec3 scaleA = Vec3Key::Interpolate(keyFrame.Scales, m_currentTime);
						vec3 scaleB = Vec3Key::Interpolate(nextKeyFrame.Scales, m_nextTime);

					
						pos = mix(posA, posB, t);
						rot = slerp(rotA, rotB, t);
						scale = mix(scaleA, scaleB,t);
					}
					else {
						pos = Vec3Key::Interpolate(keyFrame.Positions, m_currentTime);
						rot = QuaternionKey::Interpolate(keyFrame.Rotations, m_currentTime);
						scale = Vec3Key::Interpolate(keyFrame.Scales, m_currentTime);
					}
					

					transform->SetLocalPosition(pos);
					transform->SetLocalRotation(rot);
					transform->SetLocalScale(scale);
				}
			}
		}

		if (m_globalModelSpace.capacity() < n)
			m_globalModelSpace.resize(n, matrix4(1.0f));

		//for (size_t i = 0; i < n; ++i) {
		//	if (!boneEntities[i]) 
		//		continue;
		//	const Bone& bone = skeleton[i];
		//	matrix4 local = boneEntities[i]->GetTransform()->GetLocalMatrix();
		//	if (bone.ParentID >= 0 && boneEntities[bone.ParentID]) {
		//		m_globalModelSpace[i] = m_globalModelSpace[bone.ParentID] * local;
		//	}
		//	else {
		//		m_globalModelSpace[i] = local;
		//	}


		//}

		bool processed = false;
		for (auto& [uuid, data] : m_renderers) {
			MeshRenderer* renderer = data.Renderer;
			if (!renderer) continue;

			auto mesh = renderer->GetMesh();
			if (!mesh) continue;


			if (data.FinalBoneMatrices.capacity() < n)
				data.FinalBoneMatrices.resize(n);

			for (size_t i = 0; i < n; ++i) {

				if (!processed && boneEntities[i]) {
					const Bone& bone = skeleton[i];
					matrix4 local = boneEntities[i]->GetTransform()->GetLocalMatrix();

					if (bone.ParentID >= 0 && boneEntities[bone.ParentID]) {
						m_globalModelSpace[i] = m_globalModelSpace[bone.ParentID] * local;
					}
					else {
						m_globalModelSpace[i] = local;
					}
				}

				data.FinalBoneMatrices[i] = globalInverse * m_globalModelSpace[i] * skeleton[i].OffsetMatrix;
			}
			processed = true;
		}
	}
}