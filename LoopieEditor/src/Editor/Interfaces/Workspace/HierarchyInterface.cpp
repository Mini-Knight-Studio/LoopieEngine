#include "HierarchyInterface.h"
#include "Loopie/Core/Log.h"
#include "Loopie/Components/MeshRenderer.h"
#include "Loopie/Resources/ResourceManager.h"
#include "Loopie/Importers/MeshImporter.h"
#include "Loopie/Components/RectTransform.h"
#include "Loopie/Components/Canvas.h"
#include "Loopie/Components/Image.h"
#include "Loopie/Components/Text.h"
#include "Loopie/Components/Button.h"
#include "Loopie/Components/CanvasScaler.h"
#include "Loopie/Components/Light.h"

#include "Editor/Interfaces/Workspace/SceneInterface.h"
#include <imgui.h>

namespace Loopie {
	std::weak_ptr<Entity> HierarchyInterface::s_SelectedEntity;
	std::weak_ptr<Entity> HierarchyInterface::s_PendingSelectEntity;
	Event<OnEntityOrFileNotification> HierarchyInterface::s_OnEntitySelected;

	HierarchyInterface::HierarchyInterface() {

	}

	void HierarchyInterface::Update(const InputEventManager& inputEvent)
	{
		if (m_focused)
			HotKeysSelectedEntiy(inputEvent);
	}

	void HierarchyInterface::Render() {

		if (ImGui::Begin("Hierarchy")) {

			m_focused = ImGui::IsWindowHovered();

			if (!m_scene) {
				ImGui::End();
				return;
			}

			ImVec2 cursorPos = ImGui::GetCursorPos();

			/*if (ImGui::IsWindowHovered() && (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)))
				SelectEntity(nullptr);*/

			if (ImGui::BeginPopupContextWindow("HierarchyBackgroundContext", ImGuiPopupFlags_MouseButtonRight)) {
				DrawContextMenu(nullptr);
				ImGui::EndPopup();
			}

			const auto rootChildren = m_scene->GetRootEntity()->GetChildren();
			for (int i = 0; i < (int)rootChildren.size(); i++)
			{
				if (!rootChildren[i])
					continue;
				DrawInsertionZone(m_scene->GetRootEntity(), i);
				DrawEntitySlot(rootChildren[i], rootChildren[i]->GetIsActiveInHierarchy());
			}
			DrawInsertionZone(m_scene->GetRootEntity(), (int)rootChildren.size());

			if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
			{
				if (auto pending = s_PendingSelectEntity.lock())
					SelectEntity(pending);
				s_PendingSelectEntity.reset();
			}
		}

		if (m_showRenamePopup)
		{
			ImGui::OpenPopup("Rename");
			m_showRenamePopup = false;
		}

		// Draw the Modal
		if (ImGui::BeginPopupModal("Rename", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Enter new name:");
			ImGui::InputText("##new_name", m_renameBuffer, IM_ARRAYSIZE(m_renameBuffer));

			if (ImGui::Button("Confirm", ImVec2(120, 0)))
			{
				if (m_renameChilds) {
					if (auto target = m_renameTargetEntity.lock())
					{
						const auto& children = target->GetChildren();
						for (size_t i = 0; i < children.size(); i++)
						{
							if (children[i])
							{
								std::string newName = std::string(m_renameBuffer) + "_" + std::to_string(i);
								children[i]->SetName(newName);
							}
						}
					}
				}
				else {
					if (auto target = m_renameTargetEntity.lock())
					{
						target->SetName(std::string(m_renameBuffer));
					}
				}

				
				ImGui::CloseCurrentPopup();
				m_renameTargetEntity.reset();
			}

			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();

			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();
				m_renameTargetEntity.reset();
			}

			ImGui::EndPopup();
		}

		ImGui::End();
	}

	void HierarchyInterface::SetScene(Scene* scene)
	{
		m_scene = scene;
	}

	void HierarchyInterface::SelectEntity(std::shared_ptr<Entity> entity)
	{
		s_SelectedEntity = entity;
		s_OnEntitySelected.Notify(OnEntityOrFileNotification::OnEntitySelect);
	}

	void HierarchyInterface::DrawEntitySlot(const std::shared_ptr<Entity> entity, bool active)
	{
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
		const auto children = entity->GetChildren();
		bool hasChildren = !children.empty();

		if (!hasChildren)
			flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		if (s_SelectedEntity.lock() == entity)
			flags |= ImGuiTreeNodeFlags_Selected;

		bool isActive = active && entity->GetIsActiveInHierarchy();
		if (!isActive)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
		}

		bool opened = ImGui::TreeNodeEx((void*)entity.get(), flags, entity->GetName().c_str());

		if (!isActive)
		{
			ImGui::PopStyleColor();
		}

		if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && !ImGui::IsItemToggledOpen())
		{
			s_PendingSelectEntity = entity;
		}

		bool dragStarted = false;
		Drag(entity, dragStarted);
		if (dragStarted)
			s_PendingSelectEntity.reset();
		Drop(entity);

		if (ImGui::BeginPopupContextItem())
		{
			DrawContextMenu(entity);
			ImGui::EndPopup();
		}

		if (opened && hasChildren)
		{
			for (int i = 0; i < (int)children.size(); i++)
			{
				DrawInsertionZone(entity, i);
				DrawEntitySlot(children[i], isActive);
			}
			DrawInsertionZone(entity, (int)children.size());
			ImGui::TreePop();
		}
	}

	void HierarchyInterface::DrawInsertionZone(const std::shared_ptr<Entity> parent, int insertIndex)
	{
		ImGui::PushID((parent->GetUUID().Get() + "_iz_" + std::to_string(insertIndex)).c_str());

		ImVec2 cursorPos = ImGui::GetCursorPos();
		cursorPos.y -= 4;
		ImGui::SetCursorPos(cursorPos);

		ImVec2 zoneSize = ImVec2(ImGui::GetContentRegionAvail().x, 3.0f);
		ImGui::SetNextItemAllowOverlap();
		ImGui::InvisibleButton("##iz", zoneSize);

		if (ImGui::BeginDragDropTarget())
		{
			ImDrawList* drawList = ImGui::GetWindowDrawList();
			ImVec2 pMin = ImGui::GetItemRectMin();
			ImVec2 pMax = ImGui::GetItemRectMax();
			float midY = (pMin.y + pMax.y) * 0.5f;
			const ImU32 lineColor = IM_COL32(66, 150, 250, 255);
			drawList->AddCircleFilled(ImVec2(pMin.x + 4.0f, midY), 3.0f, lineColor);
			drawList->AddLine(ImVec2(pMin.x + 4.0f, midY), ImVec2(pMax.x, midY), lineColor, 2.0f);


			ImGui::PushStyleColor(ImGuiCol_DragDropTarget, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY_ENTITY_UUID"))
			{
				std::string uuidStr(static_cast<const char*>(payload->Data));
				std::shared_ptr<Entity> dragged = m_scene->GetEntity(UUID(uuidStr));

				if (dragged && dragged != parent)
				{
					auto currentParent = dragged->GetParent().lock();
					int targetIndex = insertIndex;

					if (currentParent == parent)
					{
						int currentOrder = dragged->GetOrder();
						if (currentOrder < targetIndex)
							targetIndex--;
						parent->ReorderChild(dragged, targetIndex);
					}
					else
					{
						dragged->SetParent(parent);
						targetIndex = std::min(targetIndex, parent->GetChildCount() - 1);
						parent->ReorderChild(dragged, targetIndex);
					}
				}
			}

			ImGui::PopStyleColor();

			ImGui::EndDragDropTarget();
		}

		cursorPos = ImGui::GetCursorPos();
		cursorPos.y -= 3;
		ImGui::SetCursorPos(cursorPos);

		ImGui::PopID();
	}

	void HierarchyInterface::DrawContextMenu(const std::shared_ptr<Entity> entity)
	{

		//// EXPAND MAYBE WITH A CUSTOM CREATOR -> MenuItem class (contains an Execute function, label, active Condition)???

		if (ImGui::MenuItem("Create Empty"))
		{
			std::shared_ptr<Entity> newEntity = m_scene->CreateEntity("Entity", entity);
			SelectEntity(newEntity);
		}

		/*if (ImGui::MenuItem("Copy"))
		{

		}

		if (ImGui::MenuItem("Cut"))
		{

		}

		if (ImGui::MenuItem("Paste"))
		{

		}*/

		if (ImGui::MenuItem("Duplicate", nullptr, false, entity != nullptr))
		{
			std::shared_ptr<Entity> newEntity = m_scene->CloneEntity(entity);
			s_SelectedEntity = newEntity;
		}

		if (ImGui::MenuItem("Delete", nullptr, false, entity != nullptr))
		{
			if (s_SelectedEntity.lock() == entity)
				SelectEntity(nullptr);
			m_scene->RemoveEntity(entity->GetUUID());
		}

		ImGui::Separator();

		if (ImGui::MenuItem("Rename", nullptr, false, entity != nullptr))
		{
			m_showRenamePopup = true;
			m_renameChilds = false;
			m_renameTargetEntity = entity;
			memset(m_renameBuffer, 0, sizeof(m_renameBuffer));
		}

		if (ImGui::MenuItem("Rename Children", nullptr, false, entity != nullptr && !entity->GetChildren().empty()))
		{
			m_showRenamePopup = true;
			m_renameChilds = true;
			m_renameTargetEntity = entity;
			memset(m_renameBuffer, 0, sizeof(m_renameBuffer));
		}

		ImGui::Separator();

		if (ImGui::BeginMenu("3D Object"))
		{
			if (ImGui::MenuItem("Cube"))
				SelectEntity(CreatePrimitiveModel("assets/models/primitives/cube.fbx", "Cube", entity));

			if (ImGui::MenuItem("Sphere"))
				SelectEntity(CreatePrimitiveModel("assets/models/primitives/sphere.fbx", "Sphere", entity));

			if (ImGui::MenuItem("Cylinder"))
				SelectEntity(CreatePrimitiveModel("assets/models/primitives/cylinder.fbx", "Cylinder", entity));

			if (ImGui::MenuItem("Plane"))
				SelectEntity(CreatePrimitiveModel("assets/models/primitives/plane.fbx", "Plane", entity));

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("UI"))
		{
			if (ImGui::MenuItem("Canvas"))
				SelectEntity(CreateCanvas("Canvas", entity));

			if (ImGui::MenuItem("Image"))
			{
				if (auto newImage = CreateImage("Image", entity))
					SelectEntity(newImage);
			}

			if (ImGui::MenuItem("Text"))
			{
				if (auto newText = CreateText("Text", entity))
					SelectEntity(newText);
			}

			if (ImGui::MenuItem("Button"))
			{
				if (auto newButton = CreateButton("Button", entity))
					SelectEntity(newButton);
			}

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Light"))
		{
			if (ImGui::MenuItem("Ambient"))
				SelectEntity(CreateLight(LightType::Ambient, "Ambient Light", entity));

			if (ImGui::MenuItem("Directional"))
				SelectEntity(CreateLight(LightType::Directional, "Directional Light", entity));

			if (ImGui::MenuItem("Point"))
				SelectEntity(CreateLight(LightType::Point, "Point Light", entity));

			if (ImGui::MenuItem("Spot"))
				SelectEntity(CreateLight(LightType::Spot, "Spot Light", entity));

			ImGui::EndMenu();
		}
	}

	void HierarchyInterface::HotKeysSelectedEntiy(const InputEventManager& inputEvent)
	{
		auto selectedEntity = s_SelectedEntity.lock();
		if (!selectedEntity) {
			return;
		}

		if (inputEvent.GetKeyStatus(SDL_SCANCODE_DELETE) == KeyState::DOWN) {
			m_scene->RemoveEntity(selectedEntity->GetUUID());
			SelectEntity(nullptr);
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

	void HierarchyInterface::Drag(const std::shared_ptr<Entity> entity, bool& dragStarted)
	{
		if (ImGui::BeginDragDropSource())
		{
			const std::string& uuidStr = entity->GetUUID().Get();

			ImGui::SetDragDropPayload("HIERARCHY_ENTITY_UUID", uuidStr.c_str(), uuidStr.size() + 1);

			ImGui::Text("%s", entity->GetName().c_str());
			ImGui::EndDragDropSource();
			dragStarted = true;
		}
	}

	void HierarchyInterface::Drop(const std::shared_ptr<Entity> entity)
	{
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload =
				ImGui::AcceptDragDropPayload("HIERARCHY_ENTITY_UUID"))
			{
				const char* uuidChars = static_cast<const char*>(payload->Data);

				std::string uuidStr(uuidChars);

				std::shared_ptr<Entity> dragged =
					m_scene->GetEntity(UUID(uuidStr));

				if (dragged && dragged != entity)
					dragged->SetParent(entity);
			}

			ImGui::EndDragDropTarget();
		}
	}


	std::shared_ptr<Entity> HierarchyInterface::CreatePrimitiveModel(const std::string& modelPath, const std::string& name, const std::shared_ptr<Entity> parent)
	{
		std::shared_ptr<Entity> newEntity = m_scene->CreateEntity(name, parent);
		MeshRenderer* renderer = newEntity->AddComponent<MeshRenderer>();

		Metadata& meta = AssetRegistry::GetOrCreateMetadata(modelPath);
		MeshImporter::ImportModel(modelPath, meta);
		std::shared_ptr<Mesh> mesh = ResourceManager::GetMesh(meta, 0);
		if (mesh)
			renderer->SetMesh(mesh);

		return newEntity;
	}

	std::shared_ptr<Entity> HierarchyInterface::CreateCanvas(const std::string& name, const std::shared_ptr<Entity> parent)
	{
		std::shared_ptr<Entity> newEntity = m_scene->CreateEntity(name, parent);
		newEntity->ReplaceTransform<RectTransform>();
		newEntity->AddComponent<Canvas>();
		newEntity->AddComponent<CanvasScaler>();

		return newEntity;
	}
	std::shared_ptr<Entity> HierarchyInterface::CreateImage(const std::string& name, const std::shared_ptr<Entity> parent)
	{
		std::shared_ptr<Entity> canvasEntity = parent;

		while (canvasEntity && !canvasEntity->GetComponent<Canvas>())
		{
			canvasEntity = canvasEntity->GetParent().lock();
		}

		if (!canvasEntity)
		{
			return nullptr;
		}

		std::shared_ptr<Entity> newEntity = m_scene->CreateEntity(name, parent);
		newEntity->ReplaceTransform<RectTransform>();
		newEntity->AddComponent<Image>();

		return newEntity;
	}
	std::shared_ptr<Entity> HierarchyInterface::CreateText(const std::string& name, const std::shared_ptr<Entity> parent)
	{
		std::shared_ptr<Entity> canvasEntity = parent;

		while (canvasEntity && !canvasEntity->GetComponent<Canvas>())
		{
			canvasEntity = canvasEntity->GetParent().lock();
		}

		if (!canvasEntity)
		{
			return nullptr;
		}

		std::shared_ptr<Entity> newEntity = m_scene->CreateEntity(name, parent);
		newEntity->ReplaceTransform<RectTransform>();
		newEntity->AddComponent<Text>();

		return newEntity;
	}
	std::shared_ptr<Entity> HierarchyInterface::CreateButton(const std::string& name, const std::shared_ptr<Entity> parent)
	{
		std::shared_ptr<Entity> canvasEntity = parent;

		while (canvasEntity && !canvasEntity->GetComponent<Canvas>())
		{
			canvasEntity = canvasEntity->GetParent().lock();
		}

		if (!canvasEntity)
		{
			return nullptr;
		}

		std::shared_ptr<Entity> newEntity = m_scene->CreateEntity(name, parent);
		newEntity->ReplaceTransform<RectTransform>();
		newEntity->AddComponent<Button>();
		newEntity->AddComponent<Image>();

		return newEntity;
	}

	std::shared_ptr<Entity> HierarchyInterface::CreateLight(LightType type, const std::string& name, const std::shared_ptr<Entity> parent)
	{
		std::shared_ptr<Entity> newEntity = m_scene->CreateEntity(name, parent);
		newEntity->AddComponent<Light>(vec3(1.0f, 1.0f, 1.0f), 0.5f, type);

		return newEntity;
	}
}