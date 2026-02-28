#include "SceneInterface.h"

#include "Loopie/Core/Log.h"
#include "Loopie/Core/Application.h"
#include "Loopie/Render/Renderer.h"
#include "Loopie/Components/Transform.h"

#include "Loopie/Resources/AssetRegistry.h"
#include "Loopie/Resources/ResourceManager.h"
#include "Loopie/Importers/MeshImporter.h"
#include "Loopie/Importers/MaterialImporter.h"
#include "Loopie/Importers/TextureImporter.h"
#include "Loopie/Components/MeshRenderer.h"


#include "Editor/Interfaces/Workspace/HierarchyInterface.h"

#include <limits>
#include <imgui.h>
#include <ImGuizmo.h>


namespace Loopie {

	SceneInterface::SceneInterface() {
		m_camera = std::make_shared<OrbitalCamera>();
		m_buffer = std::make_shared<FrameBuffer>(1,1);
		m_camera->GetCamera()->GetTransform()->SetPosition({ 0,5,-10.f });

		std::vector<std::string> iconsToLoad = {
			"assets/icons/icon_move.png",
			"assets/icons/icon_rotate.png",
			"assets/icons/icon_scale.png",
			"assets/icons/icon_trs.png"
		};

		std::vector<Metadata> iconsToLoadMetadatas;
		for (size_t i = 0; i < iconsToLoad.size(); i++)
		{
			Metadata& meta = AssetRegistry::GetOrCreateMetadata(iconsToLoad[i]);
			TextureImporter::ImportImage(iconsToLoad[i], meta);
			iconsToLoadMetadatas.emplace_back(meta);
		}

		m_moveIcon = ResourceManager::GetTexture(iconsToLoadMetadatas[0]);
		m_rotateIcon = ResourceManager::GetTexture(iconsToLoadMetadatas[1]);
		m_scaleIcon = ResourceManager::GetTexture(iconsToLoadMetadatas[2]);
		m_trsIcon = ResourceManager::GetTexture(iconsToLoadMetadatas[3]);


		m_gizmoOperation = ImGuizmo::TRANSLATE;
		m_gizmoMode = ImGuizmo::WORLD;
	}

	void SceneInterface::Update(const InputEventManager& inputEvent)
	{
		
		if (!m_focused && !m_interacted)
			return;
		
		m_camera->ProcessEvent(inputEvent);
		m_camera->Update();

		if (inputEvent.GetMouseButtonStatus(0) == KeyState::DOWN && !m_usingGuizmo)
			MousePick();

		HotKeysBasic(inputEvent);
		HotKeysSelectedEntiy(inputEvent);
	}

	void SceneInterface::Render() {
		m_usingGuizmo = ImGuizmo::IsOver() || ImGuizmo::IsUsing();
		ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoNav;
		if (m_usingGuizmo)
		{
			flags |= ImGuiWindowFlags_NoMove;
		}

		if (ImGui::Begin("Scene",nullptr, flags)) {
			m_visible = true;
			ImVec2 size = ImGui::GetContentRegionAvail();
			m_windowSize = { (int)size.x, (int)size.y };
			m_focused = ImGui::IsWindowHovered();
			ImVec2 cursorScreenPos = ImGui::GetCursorScreenPos();
			ImVec2 cursorPos = ImGui::GetCursorPos();
			ImVec2 mousePos = ImGui::GetMousePos();
			ImVec2 imageMin = cursorScreenPos;      // top-left of the Image
			ImVec2 imageMax = ImVec2(imageMin.x + size.x, imageMin.y + size.y);
			m_mousePosition = ivec2(mousePos.x - imageMin.x, mousePos.y - imageMin.y);

			if (!m_focused && m_interacted || m_focused)
				m_interacted = ImGui::IsMouseDown(ImGuiMouseButton_Left) || ImGui::IsMouseDown(ImGuiMouseButton_Right) || ImGui::IsMouseDown(ImGuiMouseButton_Middle);
			else 
				m_interacted = false;
			ImGui::Image((ImTextureID)m_buffer->GetTextureId(), size, ImVec2(0,1), ImVec2(1,0));

			Drop();

			ImGui::SetCursorPos(cursorPos);
			DrawHelperBar();

			auto selectedEntity = HierarchyInterface::s_SelectedEntity.lock();
			if (selectedEntity) {
				auto transform = selectedEntity->GetTransform();

				glm::mat4 worldMatrix = transform->GetLocalToWorldMatrix();

				Renderer::DisableDepth();
				ImGuizmo::SetRect(cursorScreenPos.x, cursorScreenPos.y, (float)m_windowSize.x, (float)m_windowSize.y);
				ImGuizmo::SetDrawlist();
				if (ImGuizmo::Manipulate(&m_camera->GetCamera()->GetViewMatrix()[0][0], &m_camera->GetCamera()->GetProjectionMatrix()[0][0], (ImGuizmo::OPERATION)m_gizmoOperation, (ImGuizmo::MODE)m_gizmoMode, &worldMatrix[0][0])) {
					transform->SetWorldMatrix(worldMatrix);
					Application::GetInstance().GetScene().GetOctree().Rebuild();				
				}
				Renderer::EnableDepth();
			}

		}else
			m_visible = false;
		ImGui::End();
	}

	void SceneInterface::StartScene()
	{
		m_buffer->Bind();

		ivec2 textureSize = ivec2(m_buffer->GetWidth(), m_buffer->GetHeight());
		Renderer::SetViewport(0, 0, (unsigned int)m_windowSize.x, (unsigned int)m_windowSize.y);
		if (m_windowSize.x != textureSize.x || m_windowSize.y != textureSize.y) {
			m_camera->GetCamera()->SetViewport(0, 0, m_windowSize.x, m_windowSize.y);
			m_buffer->Resize(m_windowSize.x, m_windowSize.y);
		}

		m_buffer->Clear();	
	}

	void SceneInterface::EndScene()
	{
		m_buffer->Unbind();
	}

	void SceneInterface::HotKeysBasic(const InputEventManager& inputEvent)
	{
		if (!m_camera->IsMoving()) {
			if (inputEvent.GetKeyStatus(SDL_SCANCODE_W) == KeyState::DOWN)
				m_gizmoOperation = (int)ImGuizmo::TRANSLATE;
			if (inputEvent.GetKeyStatus(SDL_SCANCODE_E) == KeyState::DOWN)
				m_gizmoOperation = (int)ImGuizmo::ROTATE;
			if (inputEvent.GetKeyStatus(SDL_SCANCODE_R) == KeyState::DOWN)
				m_gizmoOperation = (int)ImGuizmo::SCALE;
			if (inputEvent.GetKeyStatus(SDL_SCANCODE_T) == KeyState::DOWN)
				m_gizmoOperation = (int)ImGuizmo::UNIVERSAL;
		}
		
	}

	void SceneInterface::HotKeysSelectedEntiy(const InputEventManager& inputEvent)
	{
		auto selectedEntity = HierarchyInterface::s_SelectedEntity.lock();
		if (!selectedEntity) {
			return;
		}

		if (inputEvent.GetKeyStatus(SDL_SCANCODE_DELETE) == KeyState::DOWN) {
			Application::GetInstance().GetScene().RemoveEntity(selectedEntity->GetUUID());
			HierarchyInterface::SelectEntity(nullptr);
		}

		if (inputEvent.GetKeyWithModifier(SDL_SCANCODE_C, KeyModifier::CTRL)) {
			/// Copy
		}

		if (inputEvent.GetKeyWithModifier(SDL_SCANCODE_V, KeyModifier::CTRL)) {
			/// Paste
		}

		if (inputEvent.GetKeyWithModifier(SDL_SCANCODE_X, KeyModifier::CTRL)) {
			/// Cut
		}
	}

	void SceneInterface::Drop()
	{
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_EXPLORER_FILE"))
			{
				std::string path = (const char*)payload->Data;
				if (TextureImporter::CheckIfIsImage(path.c_str())) {
					ChargeTexture(path);
				}
				else if (MeshImporter::CheckIfIsModel(path.c_str())) {
					ChargeModel(path);
				}
				else if (MaterialImporter::CheckIfIsMaterial(path.c_str())) {
					ChargeMaterial(path);
				}
			}
			ImGui::EndDragDropTarget();
		}
	}
	void SceneInterface::DrawHelperBar()
	{
		
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 8));
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));

		int currentMode = (m_gizmoMode == ImGuizmo::WORLD) ? 0 : 1;
		ImGui::SetNextItemWidth(65);
		if (ImGui::Combo("##mode", &currentMode, m_gizmoModes.data(), (int)m_gizmoModes.size()))
			m_gizmoMode = (currentMode == 0) ? (int)ImGuizmo::WORLD : (int)ImGuizmo::LOCAL;

		ImGui::SameLine();
		ImGui::Text("	Movement Scale: %.1f", m_camera->GetMovementScale());

		bool haStyle = AddStyleGizmoOperationButton((int)ImGuizmo::TRANSLATE);
		if (ImGui::ImageButton("move",(ImTextureID)m_moveIcon->GetRendererId(), ImVec2(15, 15)))
			m_gizmoOperation = (int)ImGuizmo::TRANSLATE;
		RemoveStyleGizmoOperationButton(haStyle);

		haStyle = AddStyleGizmoOperationButton((int)ImGuizmo::ROTATE);
		if (ImGui::ImageButton("rotate", (ImTextureID)m_rotateIcon->GetRendererId(), ImVec2(15, 15)))
			m_gizmoOperation = (int)ImGuizmo::ROTATE;
		RemoveStyleGizmoOperationButton(haStyle);
		
		haStyle = AddStyleGizmoOperationButton((int)ImGuizmo::SCALE);
		if (ImGui::ImageButton("scale", (ImTextureID)m_scaleIcon->GetRendererId(), ImVec2(15, 15)))
			m_gizmoOperation = (int)ImGuizmo::SCALE;
		RemoveStyleGizmoOperationButton(haStyle);
		
		haStyle = AddStyleGizmoOperationButton((int)ImGuizmo::UNIVERSAL);
		if (ImGui::ImageButton("all", (ImTextureID)m_trsIcon->GetRendererId(), ImVec2(15, 15)))
			m_gizmoOperation = (int)ImGuizmo::UNIVERSAL;
		RemoveStyleGizmoOperationButton(haStyle);

		if(!m_usingGuizmo)
			m_usingGuizmo = ImGui::IsAnyItemHovered();

		ImGui::PopStyleVar(2);
	}

	Ray SceneInterface::MouseRay()
	{
		float ndcX = (2.0f * m_mousePosition.x) / m_buffer->GetWidth() - 1.0f;
		float ndcY = 1.0f - (2.0f * m_mousePosition.y) / m_buffer->GetHeight(); // flip Y for GL

		vec4 rayClipNear(ndcX, ndcY, -1.0f, 1.0f);
		vec4 rayClipFar(ndcX, ndcY, 1.0f, 1.0f);

		matrix4 invViewProj = glm::inverse(m_camera->GetCamera()->GetViewProjectionMatrix());

		vec4 rayWorldNear = invViewProj * rayClipNear;
		rayWorldNear /= rayWorldNear.w;

		vec4 rayWorldFar = invViewProj * rayClipFar;
		rayWorldFar /= rayWorldFar.w;

		vec3 origin = vec3(rayWorldNear);
		vec3 end = vec3(rayWorldFar);

		return Ray{ origin, normalize(end - origin), std::numeric_limits<float>::max()};
	}

	void SceneInterface::RemoveStyleGizmoOperationButton(bool hasStyle)
	{
		if (hasStyle)
			ImGui::PopStyleColor();
	}

	bool SceneInterface::AddStyleGizmoOperationButton(int operationType)
	{
		if (m_gizmoOperation == operationType) {
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0, 0.5, 0.75, 1.0));
			return true;
		}
		return false;
	}

	void SceneInterface::ChargeModel(const std::string& modelPath)
	{
		Metadata& meta = AssetRegistry::GetOrCreateMetadata(modelPath);
		MeshImporter::ImportModel(modelPath, meta);
		std::shared_ptr<Entity> parent;

		auto selected = HierarchyInterface::s_SelectedEntity.lock();
		if (meta.CachesPath.size() > 0) {
			parent = Application::GetInstance().GetScene().CreateEntity("ModelEntity", selected);
		}
		for (size_t i = 0; i < meta.CachesPath.size(); i++)
		{			
			std::shared_ptr<Mesh> mesh = ResourceManager::GetMesh(meta, (int)i);
			if (mesh) {
				std::shared_ptr<Entity> newEntity;
				if (!parent) {
					newEntity = Application::GetInstance().GetScene().CreateEntity(mesh->GetData().Name, selected);
				}else
					newEntity = Application::GetInstance().GetScene().CreateEntity(mesh->GetData().Name, parent);

				MeshRenderer* renderer = newEntity->AddComponent<MeshRenderer>();
				renderer->SetMesh(mesh);
				renderer->GetTransform()->SetLocalPosition(mesh->GetData().Position);
				renderer->GetTransform()->SetLocalRotation(mesh->GetData().Rotation);
				renderer->GetTransform()->SetLocalScale(mesh->GetData().Scale);
			}
		}
	}
	void SceneInterface::ChargeTexture(const std::string& texturePath)
	{
		Metadata& meta = AssetRegistry::GetOrCreateMetadata(texturePath);

		TextureImporter::ImportImage(texturePath, meta);
		std::shared_ptr<Texture> texture = ResourceManager::GetTexture(meta);
		if (texture) {
			auto selectedEntity = HierarchyInterface::s_SelectedEntity.lock();
			if (selectedEntity != nullptr) {
				MeshRenderer* renderer = selectedEntity->GetComponent<MeshRenderer>();
				if (renderer) {
					if (renderer->GetMaterial())
						renderer->GetMaterial()->SetTexture(texture);
				}
			}
			else {
				for (const auto& [uuid, entity] : Application::GetInstance().GetScene().GetAllEntities())
				{
					MeshRenderer* renderer = entity->GetComponent<MeshRenderer>();
					if (renderer) {
						if(renderer->GetMaterial())
							renderer->GetMaterial()->SetTexture(texture);
					}
				}
			}
		}
	}
	void SceneInterface::ChargeMaterial(const std::string& materialPath)
	{
		Metadata& meta = AssetRegistry::GetOrCreateMetadata(materialPath);

		MaterialImporter::ImportMaterial(materialPath, meta);
		std::shared_ptr<Material> material = ResourceManager::GetMaterial(meta);
		if (material) {
			auto selectedEntity = HierarchyInterface::s_SelectedEntity.lock();
			if (selectedEntity != nullptr) {
				MeshRenderer* renderer = selectedEntity->GetComponent<MeshRenderer>();
				if (renderer) {
					renderer->SetMaterial(material);
				}
			}
			else {
				for (const auto& [uuid, entity] : Application::GetInstance().GetScene().GetAllEntities())
				{
					MeshRenderer* renderer = entity->GetComponent<MeshRenderer>();
					if (renderer) {
						renderer->SetMaterial(material);
					}
				}
			}
		}
	}
	void SceneInterface::MousePick()
	{
		Ray mouseRay = MouseRay();
		float minDistance = std::numeric_limits<float>::max();
		std::shared_ptr<Entity> selectedEntity;

		std::vector<vec3> triVertexData;
		triVertexData.reserve(3);
		triVertexData.resize(3);
		vec3 meshHitPoint;

		std::unordered_set<std::shared_ptr<Entity>> possibleEntities;
		Application::GetInstance().GetScene().GetOctree().CollectIntersectingObjectsWithRay(mouseRay.StartPoint(), mouseRay.Direction(), possibleEntities);
		for (const auto& entity : possibleEntities)
		{
			if (!entity->GetIsActive())
				continue;
			MeshRenderer* renderer = entity->GetComponent<MeshRenderer>();
			if (!renderer || !renderer->GetIsActive() || !renderer->GetMesh())
				continue;
			const AABB& aabb = renderer->GetWorldAABB();
			if(!aabb.IntersectsRay(mouseRay.StartPoint(), mouseRay.EndPoint()))
				continue;
			const MeshData& meshData = renderer->GetMesh()->GetData();
			Triangle triangle;

			unsigned int  triangleCount = (unsigned int)meshData.Indices.size() / 3;

			for (unsigned int i = 0; i < triangleCount; i++)
			{
				if (!renderer->GetTriangle(i, triangle)) continue;
				triVertexData[0] = triangle.v0;
				triVertexData[1] = triangle.v1;
				triVertexData[2] = triangle.v2;
				if (!mouseRay.Intersects(triVertexData, true, meshHitPoint))
					continue;
				float distance = glm::distance(mouseRay.StartPoint(), meshHitPoint);
				if (distance < minDistance)
				{
					minDistance = distance;
					selectedEntity = entity;
				}
			}
		}
		HierarchyInterface::SelectEntity(selectedEntity);
	}
}