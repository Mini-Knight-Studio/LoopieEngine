#include "GameInterface.h"

#include "Loopie/Core/Log.h"
#include "Loopie/Core/Application.h"
#include "Loopie/Render/Renderer.h"

#include <imgui.h>

namespace Loopie {

	GameInterface::GameInterface() {
		m_buffer = std::make_shared<FrameBuffer>(1, 1);
	}

	void GameInterface::Render() {

		ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoNav;
		if (ImGui::Begin("Game", nullptr, flags)) {
			m_visible = true;

			ImVec2 size = ImGui::GetContentRegionAvail();
			m_windowSize = { (int)size.x, (int)size.y };

			const ImVec2 imageMin = ImGui::GetCursorScreenPos();
			const ImVec2 mouse = ImGui::GetMousePos();

			const ImVec2 local(mouse.x - imageMin.x, mouse.y - imageMin.y);
			m_mousePosGameLocal = vec2(local.x, local.y);

			m_mouseOverGame = ImGui::IsWindowHovered() && local.x >= 0.0f && local.y >= 0.0f &&
				local.x < size.x && local.y < size.y;

			ImGui::Image((ImTextureID)m_buffer->GetTextureId(), size, ImVec2(0, 1), ImVec2(1, 0));
		}
		else
		{
			m_visible = false;
			m_mouseOverGame = false;
			m_mousePosGameLocal = vec2(0.0f);
		}

		ImGui::End();
	}

	Camera* GameInterface::GetCamera()
	{
		return Camera::GetMainCamera();
	}

	void GameInterface::StartScene()
	{
		m_buffer->Bind();

		if (!Camera::GetMainCamera()) {
			m_buffer->Clear();
			return;
		}

		ivec2 textureSize = ivec2(m_buffer->GetWidth(), m_buffer->GetHeight());
		vec4 viewportSize = Camera::GetMainCamera()->GetViewport();
		Renderer::SetViewport(0, 0, (unsigned int)m_windowSize.x, (unsigned int)m_windowSize.y);

		if (m_windowSize.x != textureSize.x || m_windowSize.y != textureSize.y)
			m_buffer->Resize(m_windowSize.x, m_windowSize.y);
		if(m_windowSize.x != viewportSize.z || m_windowSize.y != viewportSize.w)
			Camera::GetMainCamera()->SetViewport(0, 0, m_windowSize.x, m_windowSize.y);

		m_buffer->Clear();
	}

	void GameInterface::EndScene()
	{
		m_buffer->Unbind();
	}
}