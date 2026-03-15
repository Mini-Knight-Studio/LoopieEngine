#pragma once
#include "Loopie/Math/MathTypes.h"
#include "Loopie/Math/Frustum.h"
#include "Loopie/Components/Component.h"
#include "Loopie/Events/EventTypes.h"
#include "Loopie/Render/FrameBuffer.h"

namespace Loopie
{
	enum class CameraProjection
	{
		Perspective = 0,
		Orthographic
	};

	class Camera : public Component, public IObserver<TransformNotification>
	{
	public:
		DEFINE_TYPE(Camera)

		Camera(float fov = 60.0f, float near_plane = 0.3f, float far_plane = 200.0f, bool canBeMainCamera = true);
		~Camera();
		void Init() override; //// From Component
		void RenderGizmo() override;
		void OnNotify(const TransformNotification& id) override;

		void SetViewport(unsigned int x, unsigned int y, unsigned int width, unsigned int height);

		const matrix4& GetViewMatrix()const;
		const matrix4& GetProjectionMatrix()const;
		const matrix4& GetViewProjectionMatrix()const;

		void SetFov(float fov);
		float GetFov() const;

		void SetNearPlane(float nearPlane);
		float GetNearPlane() const;

		void SetFarPlane(float farPlane);
		float GetFarPlane() const;
		vec4 GetViewport() const { return m_viewport; }
		const Frustum& GetFrustum() const;

		vec3 Camera::GetPosition()const;
		vec3 Camera::GetForward() const;
		vec3 Camera::GetUp() const;

		void SetDirty() const;

		const std::shared_ptr<FrameBuffer> GetRenderTarget() const { return m_renderTarget; }
		void SetRenderTarget(std::shared_ptr<FrameBuffer> buffer) { m_renderTarget = buffer; }

		void SetProjection(CameraProjection type);
		CameraProjection GetProjection() const { return m_projectionType; }

		// Ortographic
		void SetOrthoSize(float size);
		float GetOrthoSize() const { return m_orthoSize; }

		static Camera* GetMainCamera() { return s_Main; }
		static bool SetMainCamera(Camera* camera);

		bool SetAsMainCamera();
		bool CanBeMainCamera() const { return m_canBeMainCamera; }
		void SetIfBeMainCamera(bool canBe) { m_canBeMainCamera = canBe; }
		bool IsMainCamera() { return m_isMainCamera; }

		JsonNode Serialize(JsonNode& parent) const override;
		void Deserialize(const JsonNode& data) override;

	private:
		void CalculateMatrices() const;
	private:
		vec4 m_viewport = vec4(0, 0, 1, 1);
		mutable Frustum m_frustum;

		float m_fov = 60.0f;
		float m_nearPlane = 0.3f;
		float m_farPlane = 200.0f;

		mutable matrix4 m_viewMatrix = matrix4(1);
		mutable matrix4 m_projectionMatrix = matrix4(1);
		mutable matrix4 m_viewProjectionMatrix = matrix4(1);

		mutable bool m_dirty = true;

		std::shared_ptr<FrameBuffer> m_renderTarget = nullptr;

		static Camera* s_Main;
		bool m_canBeMainCamera = true;
		bool m_isMainCamera = false;

		CameraProjection m_projectionType = CameraProjection::Perspective;
		
		// For orthographic projection
		float m_orthoSize = 20.0f;
	};
}