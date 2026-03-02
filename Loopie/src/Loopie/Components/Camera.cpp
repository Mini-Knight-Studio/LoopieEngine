#include "Camera.h"

#include "Loopie/Core/Log.h"
#include "Loopie/Math/MathUtils.h"
#include "Loopie/Scene/Entity.h"
#include "Loopie/Components/Transform.h"
#include "Loopie/Render/Renderer.h"
#include "Loopie/Render/Gizmo.h"


namespace Loopie
{
	Camera* Camera::s_Main = nullptr;

	Camera::Camera(float fov, float near_plane, float far_plane, bool canBeMainCamera) : m_fov(fov), m_nearPlane(near_plane), m_farPlane(far_plane), m_canBeMainCamera(canBeMainCamera)
	{
		m_renderTarget = nullptr;
		Renderer::RegisterCamera(*this);

		if (!s_Main && CanBeMainCamera())
			SetAsMainCamera();
	}

	Camera::~Camera() {
		Renderer::UnregisterCamera(*this);

		if (GetOwner() && GetTransform())
			GetTransform()->m_transformNotifier.RemoveObserver(this);

		if (s_Main == this)
		{
			s_Main = nullptr;
			/// FindNewCamera
			const std::vector<Camera*>& cameras = Renderer::GetRendererCameras();
			for (const auto camera : cameras) {
				if (camera->CanBeMainCamera())
				{
					SetMainCamera(camera);
					return;
				}
			}
		}
	}

	void Camera::Init()
	{
		CalculateMatrices();

		GetTransform()->m_transformNotifier.AddObserver(this);
	}

	void Camera::RenderGizmo()
	{
		Gizmo::DrawFrustum(GetFrustum());
	}

	void Camera::OnNotify(const TransformNotification& id)
	{
		if (id == TransformNotification::OnDirty)
			SetDirty();
	}

	void Camera::SetViewport(unsigned int x, unsigned int y, unsigned int width, unsigned int height)
	{
		m_viewport = vec4(x, y, width, height);
		SetDirty();
	}

	const matrix4& Camera::GetViewMatrix() const
	{
		CalculateMatrices();
		return m_viewMatrix;
	}

	const matrix4& Camera::GetProjectionMatrix() const
	{
		CalculateMatrices();
		return m_projectionMatrix;
	}

	const matrix4& Camera::GetViewProjectionMatrix() const
	{
		CalculateMatrices();
		return m_viewProjectionMatrix;
	}

	void Camera::SetFov(float fov)
	{
		m_fov = fov;
		SetDirty();
	}

	float Camera::GetFov() const
	{
		return m_fov;
	}

	void Camera::SetNearPlane(float nearPlane)
	{
		m_nearPlane = nearPlane;
		SetDirty();
	}

	float Camera::GetNearPlane() const
	{
		return m_nearPlane;
	}

	void Camera::SetFarPlane(float farPlane)
	{
		m_farPlane = farPlane;
		SetDirty();
	}

	float Camera::GetFarPlane() const
	{
		return m_farPlane;
	}

	const Frustum& Camera::GetFrustum() const {
		CalculateMatrices();
		return m_frustum;
	}


	void Camera::CalculateMatrices() const
	{
		if (!m_dirty)
			return;

		auto transform = GetTransform();

		vec3 position = transform->GetPosition();
		vec3 forward = transform->Forward();
		vec3 up = transform->Up();

		// LEFT HANDED lookAt
		m_viewMatrix = glm::lookAtLH(position, position + forward, up);

		float aspect = m_viewport.z / m_viewport.w;

		if (m_projectionType == CameraProjection::Perspective)
		{
			m_projectionMatrix =
				glm::perspectiveLH(glm::radians(m_fov), aspect, m_nearPlane, m_farPlane);
		}
		else
		{
			const float halfHeight = m_orthoSize;
			const float halfWidth = halfHeight * aspect;

			m_projectionMatrix = glm::orthoLH(-halfWidth, halfWidth, -halfHeight, halfHeight, m_nearPlane, m_farPlane);
		}

		m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;

		m_frustum.FromMatrix(m_viewProjectionMatrix);

		m_dirty = false;
	}


	void Camera::SetDirty() const {
		m_dirty = true;
	}

	bool Camera::SetMainCamera(Camera* camera) {
		if (!camera) {
			if (s_Main)
				s_Main->m_isMainCamera = false;
			s_Main = nullptr;
			return true;
		}
		else if (camera->m_canBeMainCamera) {
			if (s_Main)
				s_Main->m_isMainCamera = false;
			s_Main = camera;
			s_Main->m_isMainCamera = true;
			camera->SetDirty();
			return true;
		}
		else
			Log::Warn("Camera cannot be MainCamera");
		return false;
	}

	bool Camera::SetAsMainCamera() {
		return SetMainCamera(this);
	}

	JsonNode Camera::Serialize(JsonNode& parent) const
	{
		JsonNode cameraObj = parent.CreateObjectField("camera");
		cameraObj.CreateField<bool>("is_main_camera", m_isMainCamera);
		cameraObj.CreateField<float>("fov", m_fov);
		cameraObj.CreateField<float>("near_plane", m_nearPlane);
		cameraObj.CreateField<float>("far_plane", m_farPlane);
		cameraObj.CreateField<int>("projection", (int)m_projectionType);
		cameraObj.CreateField<float>("ortho_size", m_orthoSize);

		return cameraObj;
	}

	void Camera::Deserialize(const JsonNode& data)
	{
		m_fov = data.GetValue<float>("fov", 60.0f).Result;
		m_nearPlane = data.GetValue<float>("near_plane", 0.1f).Result;
		m_farPlane = data.GetValue<float>("far_plane", 100.0f).Result;
		m_isMainCamera = data.GetValue<bool>("is_main_camera", false).Result;
		m_projectionType = (CameraProjection)data.GetValue<int>("projection", (int)CameraProjection::Perspective).Result;
		m_orthoSize = data.GetValue<float>("ortho_size", 20.0f).Result;
			
		if (m_isMainCamera)
		{
			SetAsMainCamera();
		}
	}

	void Camera::SetProjection(CameraProjection type)
	{
		if (m_projectionType != type)
		{
			m_projectionType = type;
			SetDirty();
		}
	}

	void Camera::SetOrthoSize(float size)
	{
		m_orthoSize = glm::max(0.0001f, size);
		SetDirty();
	}
}