#include "Animator.h"

#include "Loopie/Components/MeshRenderer.h"
#include "Loopie/Components/Transform.h"
#include "Loopie/Core/Time.h"
#include "Loopie/Core/Log.h"
#include "Loopie/Render/Gizmo.h"

#include <algorithm>

namespace Loopie {

	Animator::~Animator() {
		SetMeshRenderer(nullptr);
	}

	void Animator::Init()
	{
		// Nothing to initialize for now
	}

	void Animator::SetMeshRenderer(MeshRenderer* meshRenderer)
	{
		Stop();
		if(m_meshRenderer)
			m_meshRenderer->SetLinkedAnimator(nullptr);
		m_meshRenderer = meshRenderer;
		if (m_meshRenderer)
			m_meshRenderer->SetLinkedAnimator(this);
	}

	int Animator::GetCurrentClipIndex() const
	{
		return m_currentClipIndex;
	}

	const AnimationClip* Animator::GetClipByIndex(int index) const
	{
		if (m_meshRenderer == nullptr) {
			return nullptr;
		}
		auto mesh = m_meshRenderer->GetMesh();
		if (mesh == nullptr) {
			return nullptr;
		}

		if (index < 0 || index >= static_cast<int>(mesh->GetData().AnimationClips.size())) {
			return nullptr;
		}

		return &mesh->GetData().AnimationClips[index];
	}

	void Animator::Play(const std::string& clipName)
	{
		if (m_meshRenderer == nullptr) {
			return;
		}
		auto mesh = m_meshRenderer->GetMesh();
		if (mesh == nullptr) {
			return;
		}

		int index = 0;
		for(const auto& clip : mesh->GetData().AnimationClips) {
			if (clip.Name == clipName) {

				m_currentClip = &clip;
				m_currentTime = 0.0f;
				m_isPlaying = true;
				m_finalBoneMatrices.resize(m_meshRenderer->GetMesh()->GetData().Skeleton.size(),matrix4(1.0f));
				break;
			}
			index++;
		}
		m_currentClipIndex = index;
	}

	void Animator::Stop()
	{
		m_isPlaying = false;
		m_currentClip = nullptr;
		m_currentTime = 0.0f;
	}

	void Animator::Update()
	{
		if (!m_currentClip || !m_meshRenderer)
			return;

		auto mesh = m_meshRenderer->GetMesh();
		if (mesh == nullptr) {
			return;
		}

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

	JsonNode Animator::Serialize(JsonNode& parent) const
	{
		return JsonNode();
	}

	void Animator::Deserialize(const JsonNode& data)
	{

	}

	void Animator::CalculateBoneTransform()
	{
		if (!m_currentClip || !m_meshRenderer)
			return;

		auto mesh = m_meshRenderer->GetMesh();
		if (!mesh)
			return;

		const auto& skeleton = mesh->GetData().Skeleton;
		size_t n_bones = skeleton.size();

		m_finalBoneMatrices.resize(n_bones, matrix4(1.0f));
		std::vector<matrix4> localBoneMatrices(n_bones, matrix4(1.0f));

		matrix4 sceneRootTransform = GetTransform()->GetLocalToWorldMatrix();

		for (size_t i = 0; i < n_bones; ++i)
		{
			const auto& bone = skeleton[i];

			matrix4 parentTransform = matrix4(1.0f);
			if (bone.ParentID >= 0)
			{
				parentTransform = localBoneMatrices[bone.ParentID];
			}

			auto it = m_currentClip->KeyFrames.find(bone.Name);
			matrix4 animTransform = matrix4(1.0f);
			if (it != m_currentClip->KeyFrames.end())
			{
				const KeyFrame& keyFrame = it->second;

				vec3 interpolatedPosition = Vec3Key::Interpolate(keyFrame.Positions, m_currentTime);
				quaternion interpolatedRotation = QuaternionKey::Interpolate(keyFrame.Rotations, m_currentTime);
				vec3 interpolatedScale = Vec3Key::Interpolate(keyFrame.Scales, m_currentTime);

				matrix4 translation = glm::translate(matrix4(1.0f), interpolatedPosition);
				matrix4 rotation = glm::toMat4(interpolatedRotation);
				matrix4 scale = glm::scale(matrix4(1.0f), interpolatedScale);

				animTransform = translation * rotation * scale;
			}

			localBoneMatrices[i] = parentTransform * animTransform;

			m_finalBoneMatrices[i] = localBoneMatrices[i] * bone.OffsetMatrix;
		}
	}
}