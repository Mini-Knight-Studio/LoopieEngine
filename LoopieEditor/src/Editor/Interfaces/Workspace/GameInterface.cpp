#include "GameInterface.h"

#include "Loopie/Core/Log.h"
#include "Loopie/Core/Application.h"
#include "Loopie/Render/Renderer.h"

#include "Loopie/Resources/AssetRegistry.h"
#include "Loopie/Resources/ResourceManager.h"
#include "Loopie/Importers/TextureImporter.h"

#include <imgui.h>

namespace Loopie {

	GameInterface::GameInterface() {
		m_hdrBuffer = std::make_shared<FrameBuffer>(1, 1, Loopie::FrameBuffer::FrameBufferFormat::RGBA16F);
		m_ldrBuffer = std::make_shared<FrameBuffer>(1, 1, Loopie::FrameBuffer::FrameBufferFormat::RGBA8);

		std::vector<std::string> iconsToLoad = {
			"assets\\icons\\icon_debug.png"
		};

		std::vector<Metadata> iconsToLoadMetadatas;
		for (size_t i = 0; i < iconsToLoad.size(); i++)
		{
			Metadata& meta = AssetRegistry::GetOrCreateMetadata(iconsToLoad[i]);
			TextureImporter::ImportImage(iconsToLoad[i], meta);
			iconsToLoadMetadatas.emplace_back(meta);
		}

		m_gizmoIcon = ResourceManager::GetTexture(iconsToLoadMetadatas[0]);
	}

	void GameInterface::Render() {

		ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoNav;
		if (ImGui::Begin("Game", nullptr, flags)) {
			m_visible = true;

			ImVec2 size = ImGui::GetContentRegionAvail();
			m_windowSize = { (int)size.x, (int)size.y };

			ImVec2 cursorPos = ImGui::GetCursorPos();

			const ImVec2 imageMin = ImGui::GetCursorScreenPos();
			const ImVec2 mouse = ImGui::GetMousePos();

			const ImVec2 local(mouse.x - imageMin.x, mouse.y - imageMin.y);
			m_mousePosGameLocal = vec2(local.x, local.y);

			m_mouseOverGame = ImGui::IsWindowHovered() && local.x >= 0.0f && local.y >= 0.0f &&
				local.x < size.x && local.y < size.y;

			if(GetCamera())
				ImGui::Image((ImTextureID)m_ldrBuffer->GetTextureId(), size, ImVec2(0, 1), ImVec2(1, 0));

			ImGui::SetCursorPos(cursorPos);
			DrawHelperBar();
		}
		else
		{
			m_visible = false;
			m_mouseOverGame = false;
			m_mousePosGameLocal = vec2(0.0f);
			ImVec2 size = ImGui::GetContentRegionAvail();
			m_windowSize = { (int)size.x, (int)size.y };
		}

		ImGui::End();
	}

	Camera* GameInterface::GetCamera()
	{
		return Camera::GetMainCamera();
	}

	void GameInterface::StartScene()
	{
		m_hdrBuffer->Bind();

		if (!Camera::GetMainCamera()) {
			m_hdrBuffer->Clear();
			return;
		}

		ivec2 textureSize = ivec2(m_hdrBuffer->GetWidth(), m_hdrBuffer->GetHeight());
		vec4 viewportSize = Camera::GetMainCamera()->GetViewport();
		Renderer::SetViewport(0, 0, (unsigned int)m_windowSize.x, (unsigned int)m_windowSize.y);

		if (m_windowSize.x != textureSize.x || m_windowSize.y != textureSize.y)
		{
			m_hdrBuffer->Resize(m_windowSize.x, m_windowSize.y);
			m_ldrBuffer->Resize(m_windowSize.x, m_windowSize.y);
		}
		if(m_windowSize.x != viewportSize.z || m_windowSize.y != viewportSize.w)
			Camera::GetMainCamera()->SetViewport(0, 0, m_windowSize.x, m_windowSize.y);

		m_hdrBuffer->Clear();
	}

	void GameInterface::EndScene()
	{
		m_hdrBuffer->Unbind();
	}

	void GameInterface::PrepareFrameBuffer()
	{
		GetCamera()->SetViewport(0, 0, m_windowSize.x, m_windowSize.y);

		ivec2 textureSize = ivec2(m_hdrBuffer->GetWidth(), m_hdrBuffer->GetHeight());
		if (m_windowSize.x != textureSize.x || m_windowSize.y != textureSize.y) 
		{
			m_hdrBuffer->Resize(m_windowSize.x, m_windowSize.y);
			m_ldrBuffer->Resize(m_windowSize.x, m_windowSize.y);
		}
	}

	void GameInterface::ResolveToLDR()
	{
		Camera* cam = GetCamera();
		Renderer::BloomExtractPass(m_hdrBuffer->GetTextureId());
		Renderer::BloomDownsamplePass();
		Renderer::BloomUpsamplePass();
		Renderer::TonemapPass(m_hdrBuffer->GetTextureId(), *m_ldrBuffer, Renderer::GetBloomChain()[0]->GetTextureId(),
							  m_hdrBuffer->GetDepthId(), cam ? cam->GetProjectionMatrix() : matrix4(1.0f),
							  cam ? cam->GetViewMatrix() : matrix4(1.0f));
	}

	void GameInterface::DrawHelperBar()
	{
		ImVec2 buttonsSize = ImVec2(15, 15);
		ImVec2 framePadding = ImVec2(4, 4);
		ImVec2 itemSpacing = ImVec2(15, 8);

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, itemSpacing);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, framePadding);


		float availableWidth = ImGui::GetContentRegionAvail().x - buttonsSize.x - framePadding.x * 2;
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + availableWidth);

		bool hasStyle = m_showGizmos;
		if(hasStyle)
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0, 0.5, 0.75, 1.0));
		if (ImGui::ImageButton("show_gizmo", (ImTextureID)m_gizmoIcon->GetRendererId(), buttonsSize)) {
			m_showGizmos = !m_showGizmos;
		}
		if (hasStyle)
			ImGui::PopStyleColor();

		ImGui::PopStyleVar(2);
	}
}