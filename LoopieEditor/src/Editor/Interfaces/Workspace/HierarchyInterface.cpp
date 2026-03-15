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
	Event<OnEntityOrFileNotification> HierarchyInterface::s_OnEntitySelected;

	HierarchyInterface::HierarchyInterface() {
		
	}

	void HierarchyInterface::Update(const InputEventManager& inputEvent)
	{
		if(m_focused)
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

			if (ImGui::IsWindowHovered() && (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)))
				SelectEntity(nullptr);

			if (ImGui::BeginPopupContextWindow("HierarchyBackgroundContext", ImGuiPopupFlags_MouseButtonRight)) {
				DrawContextMenu(nullptr);
				ImGui::EndPopup();
			}

			for (const auto& entity : m_scene->GetRootEntity()->GetChildren())
			{
				if (!entity)
					continue;
				DrawEntitySlot(entity, entity->GetIsActiveInHierarchy());
				
			}
		

			ImVec2 size = ImGui::GetContentRegionAvail();
			//ImGui::SetCursorPos(cursorPos);  /// And move up the avail
			ImGui::SetNextItemAllowOverlap();
			ImGui::InvisibleButton("##DropTarget", size, ImGuiButtonFlags_None);
			Drop(m_scene->GetRootEntity());
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

		Drag(entity);
		Drop(entity);

		if (ImGui::IsItemClicked())
		{
			////Expand To Select Multiple
			SelectEntity(entity);
		}

		if (ImGui::BeginPopupContextItem())
		{
			DrawContextMenu(entity);
			ImGui::EndPopup();
		}

		if (opened && hasChildren)
		{
			for (const auto& child : children)
			{
				DrawEntitySlot(child, isActive);
			}
			ImGui::TreePop();
		}
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

		if (ImGui::MenuItem("Delete",nullptr, false, entity != nullptr))
		{
			if (s_SelectedEntity.lock() == entity)
				SelectEntity(nullptr);
			m_scene->RemoveEntity(entity->GetUUID());
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

	void HierarchyInterface::Drag(const std::shared_ptr<Entity> entity)
	{
		if (ImGui::BeginDragDropSource())
		{
			const std::string& uuidStr = entity->GetUUID().Get();

			ImGui::SetDragDropPayload("HIERARCHY_ENTITY_UUID", uuidStr.c_str(), uuidStr.size() + 1);

			ImGui::Text("%s", entity->GetName().c_str());
			ImGui::EndDragDropSource();
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