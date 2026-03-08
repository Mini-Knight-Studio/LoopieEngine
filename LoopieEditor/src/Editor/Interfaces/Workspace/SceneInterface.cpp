#include "SceneInterface.h"

#include "Loopie/Core/Log.h"
#include "Loopie/Core/Application.h"
#include "Loopie/Render/Renderer.h"
#include "Loopie/Render/Gizmo.h"
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
			"assets/icons/icon_trs.png",
			"assets/icons/icon_grid.png",
			"assets/icons/icon_octree.png",
			"assets/icons/icon_snap.png"
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
		m_gridIcon = ResourceManager::GetTexture(iconsToLoadMetadatas[4]);
		m_octreeIcon = ResourceManager::GetTexture(iconsToLoadMetadatas[5]);
		m_snapIcon = ResourceManager::GetTexture(iconsToLoadMetadatas[6]);


		m_gizmoOperation = ImGuizmo::TRANSLATE;
		m_gizmoMode = ImGuizmo::WORLD;
	}

	void SceneInterface::Update(const InputEventManager& inputEvent)
	{
		
		if (!m_focused && !m_interacted)
			return;
		
		m_camera->ProcessEvent(inputEvent);
		m_camera->Update();

		if (inputEvent.GetMouseButtonStatus(0) == KeyState::DOWN && !m_usingGuizmo) {
			if(inputEvent.GetKeyStatus(SDL_SCANCODE_LCTRL) == KeyState::REPEAT)
				MousePick(true);
			else
				MousePick();

		}

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

				bool allowSnap = (m_snapEnabled || m_temporalSnapEnabled) && (m_gizmoOperation != (int)ImGuizmo::UNIVERSAL);
				if (allowSnap) {
					switch (m_gizmoOperation)
					{
					case ImGuizmo::TRANSLATE:
						m_snap[0] = m_snap[1] = m_snap[2] = m_translationSnap;
						break;

					case ImGuizmo::ROTATE:
						m_snap[0] = m_snap[1] = m_snap[2] = m_rotationSnap;
						break;

					case ImGuizmo::SCALE:
						m_snap[0] = m_snap[1] = m_snap[2] = m_scaleSnap;
						break;
					}
				}

				if (ImGuizmo::Manipulate(&m_camera->GetCamera()->GetViewMatrix()[0][0], &m_camera->GetCamera()->GetProjectionMatrix()[0][0], (ImGuizmo::OPERATION)m_gizmoOperation, (ImGuizmo::MODE)m_gizmoMode, &worldMatrix[0][0],nullptr, allowSnap ? m_snap : nullptr)) {
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
		if (!m_camera->IsMoving() && !m_temporalSnapEnabled) {
			if (inputEvent.GetKeyStatus(SDL_SCANCODE_W) == KeyState::DOWN)
				m_gizmoOperation = (int)ImGuizmo::TRANSLATE;
			if (inputEvent.GetKeyStatus(SDL_SCANCODE_E) == KeyState::DOWN)
				m_gizmoOperation = (int)ImGuizmo::ROTATE;
			if (inputEvent.GetKeyStatus(SDL_SCANCODE_R) == KeyState::DOWN)
				m_gizmoOperation = (int)ImGuizmo::SCALE;
			if (inputEvent.GetKeyStatus(SDL_SCANCODE_T) == KeyState::DOWN)
				m_gizmoOperation = (int)ImGuizmo::UNIVERSAL;
		}
		
		
		if (inputEvent.GetKeyStatus(SDL_SCANCODE_LCTRL) == KeyState::REPEAT)
			m_temporalSnapEnabled = true;
		else
			m_temporalSnapEnabled = false;
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
				if (MeshImporter::CheckIfIsModel(path.c_str())) {
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
		ImVec2 buttonsSize = ImVec2(15, 15);
		ImVec2 framePadding = ImVec2(4, 4);
		ImVec2 itemSpacing = ImVec2(15, 8);
		
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, itemSpacing);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, framePadding);


		int currentMode = (m_gizmoMode == ImGuizmo::WORLD) ? 0 : 1;
		ImGui::SetNextItemWidth(65);
		if (ImGui::Combo("##mode", &currentMode, m_gizmoModes.data(), (int)m_gizmoModes.size()))
			m_gizmoMode = (currentMode == 0) ? (int)ImGuizmo::WORLD : (int)ImGuizmo::LOCAL;

		ImGui::SameLine();
		ImGui::Text("Movement Scale: %.1f", m_camera->GetMovementScale());
		ImGui::SameLine();

		float availableWidth = ImGui::GetContentRegionAvail().x - buttonsSize.x - framePadding.x*2;
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + availableWidth);
		bool hasStyle = AddStyleButton(Gizmo::GetGridVisibility());
		if (ImGui::ImageButton("grid", (ImTextureID)m_gridIcon->GetRendererId(), buttonsSize)) {
			Gizmo::SetGridVisibility(!Gizmo::GetGridVisibility());
		}
		RemoveStyleButton(hasStyle);

		hasStyle = AddStyleButton(m_gizmoOperation == (int)ImGuizmo::TRANSLATE);
		if (ImGui::ImageButton("move", (ImTextureID)m_moveIcon->GetRendererId(), buttonsSize)) {
			m_gizmoOperation = (int)ImGuizmo::TRANSLATE;
		}
		RemoveStyleButton(hasStyle);

		ImGui::SameLine();
		availableWidth = ImGui::GetContentRegionAvail().x - buttonsSize.x - framePadding.x * 2;
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + availableWidth);

		Octree& octree = Application::GetInstance().GetScene().GetOctree();
		hasStyle = AddStyleButton(octree.GetShouldDraw());
		if (ImGui::ImageButton("octree", (ImTextureID)m_octreeIcon->GetRendererId(), buttonsSize)) {
			octree.SetShouldDraw(!octree.GetShouldDraw());
		}
		RemoveStyleButton(hasStyle);

		hasStyle = AddStyleButton(m_gizmoOperation == (int)ImGuizmo::ROTATE);
		if (ImGui::ImageButton("rotate", (ImTextureID)m_rotateIcon->GetRendererId(), buttonsSize)) {
			m_gizmoOperation = (int)ImGuizmo::ROTATE;
		}
		RemoveStyleButton(hasStyle);

		if (m_gizmoOperation != (int)ImGuizmo::UNIVERSAL) {
			ImGui::SameLine();
			availableWidth = ImGui::GetContentRegionAvail().x - buttonsSize.x - framePadding.x * 2;
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + availableWidth);
			hasStyle = AddStyleButton(m_snapEnabled || m_temporalSnapEnabled);
			if (ImGui::ImageButton("snap", (ImTextureID)m_snapIcon->GetRendererId(), buttonsSize))
			{
				ImGui::OpenPopup("SnapSettings");
			}
			RemoveStyleButton(hasStyle);

			if (ImGui::BeginPopup("SnapSettings", ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Checkbox("Enable Snap", &m_snapEnabled);

				ImGui::Separator();

				ImGui::Text("Translation");
				ImGui::DragFloat("Step##trans", &m_translationSnap, 0.1f, 0.01f, 100.0f);

				ImGui::Text("Rotation");
				ImGui::DragFloat("Step##rot", &m_rotationSnap, 1.0f, 1.0f, 180.0f);

				ImGui::Text("Scale");
				ImGui::DragFloat("Step##scale", &m_scaleSnap, 0.01f, 0.001f, 10.0f);

				ImGui::EndPopup();
			}
		}
		
		hasStyle = AddStyleButton(m_gizmoOperation == (int)ImGuizmo::SCALE);
		if (ImGui::ImageButton("scale", (ImTextureID)m_scaleIcon->GetRendererId(), buttonsSize)) {
			m_gizmoOperation = (int)ImGuizmo::SCALE;
		}
		RemoveStyleButton(hasStyle);

		hasStyle = AddStyleButton(m_gizmoOperation == (int)ImGuizmo::UNIVERSAL);
		if (ImGui::ImageButton("all", (ImTextureID)m_trsIcon->GetRendererId(), buttonsSize))
			m_gizmoOperation = (int)ImGuizmo::UNIVERSAL;
		RemoveStyleButton(hasStyle);

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

	void SceneInterface::RemoveStyleButton(bool hasStyle)
	{
		if (hasStyle)
			ImGui::PopStyleColor();
	}

	bool SceneInterface::AddStyleButton(bool add)
	{
		if(add)
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0, 0.5, 0.75, 1.0));
		return add;
	}

	
	void SceneInterface::ChargeModel(const std::string& modelPath)
	{
		std::filesystem::path path(modelPath);
		Metadata& meta = AssetRegistry::GetOrCreateMetadata(modelPath);
		MeshImporter::ImportModel(modelPath, meta);

		auto selected = HierarchyInterface::s_SelectedEntity.lock();
		if (meta.CachesPath.empty()) return;

		auto modelRoot = Application::GetInstance().GetScene().CreateEntity(path.stem().string(), selected);

		std::vector<std::shared_ptr<Entity>> boneEntities;
		Animator* animator = nullptr;

		for (size_t i = 0; i < meta.CachesPath.size(); ++i)
		{
			auto mesh = ResourceManager::GetMesh(meta, (int)i);
			if (mesh && !mesh->GetData().Skeleton.empty())
			{
				const auto& skeleton = mesh->GetData().Skeleton;
				boneEntities.reserve(skeleton.size());

				for (const auto& bone : skeleton)
				{
					auto boneEntity = Application::GetInstance().GetScene().CreateEntity(bone.Name, modelRoot);

					glm::vec3 position, scale;
					glm::quat rotation;
					glm::vec3 skew;
					glm::vec4 perspective;
					glm::decompose(bone.LocalBindTransform, scale, rotation, position, skew, perspective);
					rotation = glm::normalize(rotation);

					boneEntity->GetTransform()->SetLocalPosition(position);
					boneEntity->GetTransform()->SetLocalRotation(rotation);
					boneEntity->GetTransform()->SetLocalScale(scale);
					boneEntities.push_back(boneEntity);
				}

				for (size_t j = 0; j < skeleton.size(); ++j)
				{
					int parentID = skeleton[j].ParentID;
					if (parentID >= 0 && parentID < static_cast<int>(skeleton.size()))
					{
						boneEntities[j]->SetParent(boneEntities[parentID]);
					}
				}
				break;
			}
		}

		for (size_t i = 0; i < meta.CachesPath.size(); ++i)
		{
			auto mesh = ResourceManager::GetMesh(meta, (int)i);
			if (!mesh) continue;

			auto meshEntity = Application::GetInstance().GetScene().CreateEntity(mesh->GetData().Name, modelRoot);

			MeshRenderer* renderer = meshEntity->AddComponent<MeshRenderer>();
			renderer->SetMesh(mesh);
			renderer->GetTransform()->SetLocalPosition(mesh->GetData().Position);
			renderer->GetTransform()->SetLocalRotation(mesh->GetData().Rotation);
			renderer->GetTransform()->SetLocalScale(mesh->GetData().Scale);

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
	void SceneInterface::MousePick(bool getParent)
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
			if (!renderer || !renderer->GetLocalIsActive() || !renderer->GetMesh())
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
		if (selectedEntity && getParent) {
			std::shared_ptr<Entity> parent = selectedEntity->GetParent().lock();
			if(parent && parent != Application::GetInstance().GetScene().GetRootEntity())
				selectedEntity = parent;
		}


		HierarchyInterface::SelectEntity(selectedEntity);
	}
}