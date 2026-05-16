#pragma once


#include "Loopie/Resources/Types/Texture.h"

#include "Editor/Interfaces/Interface.h"
#include "Loopie/Render/FrameBuffer.h"

namespace Loopie {
	class Camera;

	class GameInterface : public Interface {
	public:
		GameInterface();
		~GameInterface() = default;
		void Init() override {}
		void Render() override;

		void StartScene();
		void EndScene();

		bool IsVisible() { return m_visible; }

		void PrepareFrameBuffer();
		std::shared_ptr<FrameBuffer> GetFrameBuffer() const { return m_buffer; }

		Camera* GetCamera();

		bool IsMouseOverGame() const { return m_mouseOverGame; }
		vec2 GetMousePosGameLocal() const { return m_mousePosGameLocal; }
		ivec2 GetGameSize() const { return m_windowSize; }

		bool ShowGizmos() const { return m_showGizmos; }
		void SetIfShowGizmos(bool show) { m_showGizmos = show; }

	private:
		void DrawHelperBar();

	private:
		bool m_visible = false;
		std::shared_ptr<FrameBuffer> m_buffer;
		ivec2 m_windowSize = ivec2(0);

		bool m_mouseOverGame = false;
		vec2 m_mousePosGameLocal = vec2(0.0f);

		bool m_showGizmos = true;

		std::shared_ptr<Texture> m_gizmoIcon;
	};
}