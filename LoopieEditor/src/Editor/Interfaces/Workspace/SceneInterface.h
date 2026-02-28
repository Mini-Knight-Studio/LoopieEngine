#pragma once 

#include "Loopie/Resources/Types/Texture.h"

#include "Editor/Interfaces/Interface.h"
#include "Editor/Others/OrbitalCamera.h"
#include "Loopie/Render/FrameBuffer.h"
#include "Loopie/Math/Ray.h"
#include <array>

namespace Loopie {

	class SceneInterface : public Interface {
	public:
		SceneInterface();
		~SceneInterface() = default;
		void Init() override {}
		void Update(const InputEventManager& inputEvent) override;
		void Render() override;

		void StartScene();
		void EndScene();

   		Camera* GetCamera() { return m_camera->GetCamera(); }

		void ChargeModel(const std::string& modelPath);
		void ChargeTexture(const std::string& texturePath);
		void ChargeMaterial(const std::string& materialPath);

		void MousePick(bool getParent = false);

		bool IsVisible() { return m_visible; }

	private:

		void HotKeysBasic(const InputEventManager& inputEvent);
		void HotKeysSelectedEntiy(const InputEventManager& inputEvent);

		void Drop();
		void DrawHelperBar();
		Ray MouseRay();

		bool AddStyleGizmoOperationButton(int operationType);
		void RemoveStyleGizmoOperationButton(bool hasStyle);


	private:

		std::shared_ptr<FrameBuffer> m_buffer;
		std::shared_ptr<OrbitalCamera> m_camera;

		bool m_usingGuizmo = false;
		bool m_interacted = false;
		bool m_visible = false;

		ivec2 m_windowSize = ivec2(0);
		ivec2 m_mousePosition = ivec2(0);

		int m_gizmoOperation = 0;
		int m_gizmoMode = 0;
		std::array<const char*,2> m_gizmoModes = { "World", "Local" };

		std::shared_ptr<Texture> m_moveIcon;
		std::shared_ptr<Texture> m_scaleIcon;
		std::shared_ptr<Texture> m_rotateIcon;
		std::shared_ptr<Texture> m_trsIcon;
	};
}