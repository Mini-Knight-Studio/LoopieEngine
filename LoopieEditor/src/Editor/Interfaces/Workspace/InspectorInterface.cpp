#include "InspectorInterface.h"
#include "Editor/Interfaces/Workspace/HierarchyInterface.h"
#include "Editor/Interfaces/Workspace/AssetsExplorerInterface.h"

#include "Loopie/Core/Log.h"
#include "Loopie/Core/Application.h"
#include "Loopie/Math/MathTypes.h"

#include "Loopie/Components/Transform.h"
#include "Loopie/Components/RectTransform.h"
#include "Loopie/Components/Camera.h"
#include "Loopie/Components/MeshRenderer.h"
#include "Loopie/Components/ScriptClass.h"
#include "Loopie/Components/Animator.h"
#include "Loopie/Components/Canvas.h"
#include "Loopie/Components/Image.h"
#include "Loopie/Components/BoxCollider.h"
#include "Loopie/Scripting/ScriptingManager.h"
#include "Loopie/Importers/TextureImporter.h"

#include "Loopie/Resources/AssetRegistry.h"
#include "Loopie/Resources/ResourceManager.h"

#include <imgui.h>
#include <unordered_map>

namespace Loopie {

	InspectorInterface::InspectorInterface() {
		
	}

	InspectorInterface::~InspectorInterface() {
		HierarchyInterface::s_OnEntitySelected.RemoveObserver(this);
		AssetsExplorerInterface::s_OnFileSelected.RemoveObserver(this);
	}

	void InspectorInterface::Init() {
		HierarchyInterface::s_OnEntitySelected.AddObserver(this);
		AssetsExplorerInterface::s_OnFileSelected.AddObserver(this);
	}


	void InspectorInterface::Render() {
		if (ImGui::Begin("Inspector")) {

			switch (m_mode)
			{
			case InspectorMode::EntityMode:
				DrawEntityInspector(HierarchyInterface::s_SelectedEntity.lock());
				break;
			case InspectorMode::ImportMode:
			{
				const std::filesystem::path& path = AssetsExplorerInterface::s_SelectedFile;
				const bool hasInspectableFile = (!path.empty() && path.has_extension() && path.extension().string() == ".mat");
				if (hasInspectableFile)
					DrawFileImportSettings(path);
				else
					DrawEntityInspector(HierarchyInterface::s_SelectedEntity.lock());

				break;
			}
			default:
				break;
			}
		}
		ImGui::End();
	}

	void InspectorInterface::DrawEntityInspector(const std::shared_ptr<Entity>& entity)
	{
		if (!entity)
			return;

		DrawEntityConfig(entity);

		std::vector<Component*> components = entity->GetComponents();
		for (auto* component : components) {
			if (component->GetTypeID() == Transform::GetTypeIDStatic()) {
				DrawTransform(static_cast<Transform*>(component));
			}
			else if (component->GetTypeID() == RectTransform::GetTypeIDStatic()) {
				DrawTransform(static_cast<RectTransform*>(component));
			}
			else if (component->GetTypeID() == Camera::GetTypeIDStatic()) {
				DrawCamera(static_cast<Camera*>(component));
			}
			else if (component->GetTypeID() == MeshRenderer::GetTypeIDStatic()) {
				DrawMeshRenderer(static_cast<MeshRenderer*>(component));
			}
			else if (component->GetTypeID() == Animator::GetTypeIDStatic()) {
				DrawAnimator(static_cast<Animator*>(component));
			}
			else if (component->GetTypeID() == ScriptClass::GetTypeIDStatic()) {
				DrawScriptClass(static_cast<ScriptClass*>(component));
			}
			else if (component->GetTypeID() == Canvas::GetTypeIDStatic()) {
				DrawCanvas(static_cast<Canvas*>(component));
			}
			else if (component->GetTypeID() == Image::GetTypeIDStatic()) {
				DrawImage(static_cast<Image*>(component));
			}
			else if (component->GetTypeID() == BoxCollider::GetTypeIDStatic()) {
				DrawBoxCollider(static_cast<BoxCollider*>(component));
			}
		}
		AddComponent(entity);
	}

	void InspectorInterface::DrawFileImportSettings(const std::filesystem::path& path)
	{
		if (path.empty() || !path.has_extension()) 
			return;

		Metadata* metadata = AssetRegistry::GetMetadata(path.string());
		if (!metadata)
			return;

		std::string extension = path.extension().string();
		if (extension == ".mat") 
		{
			std::shared_ptr<Material> material = ResourceManager::GetMaterial(*metadata);
			DrawMaterial(material);
		}
	}

	void InspectorInterface::DrawEntityConfig(const std::shared_ptr<Entity>& entity)
	{
		char nameBuffer[256];
		memset(nameBuffer, 0, sizeof(nameBuffer));
		strncpy_s(nameBuffer, entity->GetName().c_str(), sizeof(nameBuffer) - 1);

		bool isActive = entity->GetIsActiveInHierarchy();
		if (ImGui::Checkbox("##", &isActive)) {
			entity->SetIsActive(isActive);
		}

		ImGui::SameLine();
		
		if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer))) {
			entity->SetName(std::string(nameBuffer));
		}

		ImGui::SameLine();

		if (ImGui::Button("UUID"))
		{
			Application::GetInstance().m_clipboard.Copy(entity->GetUUID().Get());
			Log::Warn("{0}", Application::GetInstance().m_clipboard.Paste());
		}
		
		ImGui::Separator();
		
		ImGui::TextDisabled("UUID: %s", entity->GetUUID().Get().c_str());

		ImGui::Separator();
	}

	void InspectorInterface::DrawTransform(Transform* transform)
	{
		ImGui::PushID(transform);

		const char* label = transform->IsRectTransform() ? "Rect Transform" : "Transform";
		bool open = ImGui::CollapsingHeader(label, ImGuiTreeNodeFlags_DefaultOpen);
		bool modified = false;
		ImGui::SetItemTooltip(transform->GetUUID().Get().c_str());
		ComponentContextMenu(transform, false);

		if (open) {
			vec3 position = transform->GetLocalPosition();
			vec3 rotation = transform->GetLocalEulerAngles();
			vec3 scale = transform->GetLocalScale();

			if (ImGui::DragFloat3("Position", &position.x, 0.1f)) {
				modified = true;
				transform->SetLocalPosition(position);
			}
			if (ImGui::DragFloat3("Rotation", &rotation.x, 0.5f)) {
				modified = true;
				transform->SetLocalEulerAngles(rotation);
			}
			if (ImGui::DragFloat3("Scale", &scale.x, 0.1f)) {
				modified = true;
				transform->SetLocalScale(scale);
			}
			if (transform->IsRectTransform())
			{
				float w = transform->GetWidth();
				float h = transform->GetHeight();

				if (ImGui::DragFloat("Width", &w, 1.f))
					transform->SetWidth(w);

				if (ImGui::DragFloat("Height", &h, 1.f))
					transform->SetHeight(h);
			}
		}
		ImGui::PopID();

		if(modified)
			Application::GetInstance().GetScene().GetOctree().Rebuild();
	}

	void InspectorInterface::DrawCamera(Camera* camera)
	{
		ImGui::PushID(camera);

		bool open = ImGui::CollapsingHeader("Camera");
		ImGui::SetItemTooltip(camera->GetUUID().Get().c_str());
		if (ComponentContextMenu(camera)) {
			ImGui::PopID();
			return;
		}

		if (open) {
			float fov = camera->GetFov();
			float nearPlane = camera->GetNearPlane();
			float farPlane = camera->GetFarPlane();
			bool isMainCamera = Camera::GetMainCamera() == camera;

			CameraProjection projection = camera->GetProjection();
			int projIndex = (int)projection;
			const char* projectionLabels[] = { "Perspective", "Orthographic" };

			if (ImGui::Combo("Projection", &projIndex, projectionLabels, IM_ARRAYSIZE(projectionLabels))) {
				camera->SetProjection((CameraProjection)projIndex);
			}

			if (camera->GetProjection() == CameraProjection::Perspective)
			{
				if (ImGui::DragFloat("Fov", &fov, 1.0f, 1.0f, 179.0f))
					camera->SetFov(fov);
			}
			else
			{
				float orthoSize = camera->GetOrthoSize();
				if (ImGui::DragFloat("Ortho Size", &orthoSize, 0.1f, 0.1f, 100000.0f))
					camera->SetOrthoSize(orthoSize);
			}

			if (ImGui::DragFloat("Near Plane", &nearPlane, 0.01f, 0.01f, farPlane - 0.01f))
				camera->SetNearPlane(nearPlane);

			if (ImGui::DragFloat("Far Plane", &farPlane, 1.0f, nearPlane + 0.1f, 10000.0f))
				camera->SetFarPlane(farPlane);

			ImGui::Separator();
			if (ImGui::Checkbox("Main Camera", &isMainCamera)) {
				if(isMainCamera)
					camera->SetAsMainCamera();
			}
		}
		ImGui::PopID();
	}

	void InspectorInterface::DrawMeshRenderer(MeshRenderer* meshRenderer)
	{
		ImGui::PushID(meshRenderer);

		bool open = ImGui::CollapsingHeader("Mesh Renderer");
		ImGui::SetItemTooltip(meshRenderer->GetUUID().Get().c_str());
		if (ComponentContextMenu(meshRenderer)) {
			ImGui::PopID();
			return;
		}

		if (open) {
			auto mesh = meshRenderer->GetMesh();
			int prevMeshIndex = 0;
			int meshIndex = 0;
			if (mesh)
			{
				prevMeshIndex = mesh->GetMeshIndex();
				meshIndex = prevMeshIndex;
			}
			ImGui::Text("Mesh: %s", mesh ? "Assigned" : "None");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(80);
			ImGui::InputInt("", &meshIndex,1,0);
			if (meshIndex < 0)
				meshIndex = 0;
			ImGui::SameLine();
			if (meshIndex != prevMeshIndex && mesh)
			{
				Metadata* data = AssetRegistry::GetMetadata(mesh->GetUUID());
				if (data && data->Type == ResourceType::MESH)
				{
					int maxIndex = (int)data->CachesPath.size();
					if (meshIndex >= maxIndex)
						meshIndex = maxIndex - 1;
					std::shared_ptr<Mesh> newMesh = ResourceManager::GetMesh(*data, meshIndex);
					if (newMesh)
					{
						mesh = newMesh;
						meshRenderer->SetMesh(mesh);
					}
				}
			}
			if (ImGui::Button("Paste"))
			{
				std::string uuid = Application::GetInstance().m_clipboard.Paste();
				if (UUID::IsValid(uuid))
				{
					Metadata* data = AssetRegistry::GetMetadata(UUID(uuid));
					if (data && data->Type == ResourceType::MESH)
					{
						int maxIndex = (int)data->CachesPath.size();
						if (meshIndex >= maxIndex)
							meshIndex = maxIndex - 1;
						std::shared_ptr<Mesh> newMesh = ResourceManager::GetMesh(*data, meshIndex);
						if (newMesh)
						{
							mesh = newMesh;
							meshRenderer->SetMesh(mesh);
						}
					}
				}
			}
			if (!mesh) {
				ImGui::PopID();
				return;
			}
			ImGui::Text("Mesh Resource Count: %u", mesh->GetReferenceCount());
			ImGui::Text("Mesh Vertices: %d", mesh->GetData().VerticesAmount);
			ImGui::Text("Has Bones: %s", mesh->GetData().HasBones ? "True" : "False");

			ImGui::Separator();

			bool drawFN = meshRenderer->GetDrawNormalsPerFace();
			bool drawTN = meshRenderer->GetDrawNormalsPerTriangle();
			bool drawAABB = meshRenderer->GetDrawAABB();
			bool drawOBB = meshRenderer->GetDrawOBB();
			if (ImGui::Checkbox("Draw Face Normals", &drawFN))
				meshRenderer->SetDrawNormalsPerFace(drawFN);
			if (ImGui::Checkbox("Draw Triangle Normals", &drawTN))
				meshRenderer->SetDrawNormalsPerTriangle(drawTN);
			if (ImGui::Checkbox("Draw AABB", &drawAABB))
					meshRenderer->SetDrawAABB(drawAABB);
			if (ImGui::Checkbox("Draw OBB", &drawOBB))
				meshRenderer->SetDrawOBB(drawOBB);
			//ImGui::Text("Shader: %s", meshRenderer->GetShader().GetName().c_str()); ????

			ImGui::Separator();
			ImGui::Separator();
			std::shared_ptr<Material> material = meshRenderer->GetMaterial();
			DrawMaterial(material);
			
		}

		ImGui::PopID();
	}

	void InspectorInterface::DrawAnimator(Animator* animator)
	{
		ImGui::PushID(animator);

		bool open = ImGui::CollapsingHeader("Animator");
		ImGui::SetItemTooltip(animator->GetUUID().Get().c_str());
		if (ComponentContextMenu(animator)) {
			ImGui::PopID();
			return;
		}

		if (open) {
			ImGui::Text("Linked MeshRenderers:");
			const auto& renderers = animator->GetRenderers();

			if (renderers.empty())
			{
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "None");
			}
			else
			{
				int index = 0;
				for (const auto& [uuid, data] : renderers)
				{
					MeshRenderer* renderer = data.Renderer;
					if (!renderer) continue;

					ImGui::PushID(index++);
					std::shared_ptr<Entity> owner = renderer->GetOwner();
					auto allRenderersInEntity = owner->GetComponents<MeshRenderer>();
					int rendererIndex = 0;
					for (int i = 0; i < allRenderersInEntity.size(); i++) {
						if (allRenderersInEntity[i] == renderer) {
							rendererIndex = i;
							break;
						}
					}

					if (ImGui::SmallButton("X"))
					{
						animator->RemoveMeshRenderer(renderer);
						ImGui::PopID();
						break;
					}
					ImGui::SameLine();

					bool isTarget = (animator->GetTargetRenderer() == renderer);
					std::string label;
					if (allRenderersInEntity.size() > 1) {
						label = fmt::format("{} [{}] : {}...", owner->GetName(), rendererIndex, renderer->GetUUID().Get().substr(0, 8));
					}
					else {
						label = fmt::format("{} : {}...", owner->GetName(), renderer->GetUUID().Get().substr(0, 8));
					}

					if (isTarget)
						ImGui::TextColored(ImVec4(0, 1, 0, 1), "%s (Target)", label.c_str());
					else
						ImGui::Text("%s", label.c_str());

					ImGui::SameLine();

					if (!isTarget && ImGui::SmallButton("Set Target"))
					{
						animator->SetTargetRenderer(renderer);
					}
					if (!isTarget) 
						ImGui::SameLine();

					if (ImGui::SmallButton("Copy UUID"))
					{
						Application::GetInstance().m_clipboard.Copy(renderer->GetUUID().Get());
					}
					ImGui::PopID();
				}
			}


			ImGui::Separator();

			ImGui::Text("Add MeshRenderer:");
			static Entity* selectedEntity = nullptr;
			static MeshRenderer* selectedRenderer = nullptr;

			if (ImGui::BeginCombo("Target", selectedRenderer ? selectedRenderer->GetOwner()->GetName().c_str() : "Select..."))
			{
				std::shared_ptr<Entity> owner = animator->GetOwner()->shared_from_this();
				std::vector<Entity*> candidates;
				candidates.push_back(owner.get());

				std::function<void(Entity*)> collect = [&](Entity* e) {
					for (auto& child : e->GetChildren())
					{
						candidates.push_back(child.get());
						collect(child.get());
					}
					};
				collect(owner.get());

				for (Entity* entity : candidates)
				{
					std::vector<MeshRenderer*> renderersInEntity = entity->GetComponents<MeshRenderer>();
					if (renderersInEntity.empty()) 
						continue;

					int index = 0;
					bool requiresIndex = renderersInEntity.size() > 1;
					for (const auto renderer : renderersInEntity) {
						if (renderers.find(renderer->GetUUID()) != renderers.end()) {
							index++;
							continue;
						}
						bool isSelected = (selectedRenderer == renderer);
						std::string displayName = entity->GetName();
						if (requiresIndex) {
							displayName += fmt::format(" [{}]", index);
						}

						if (ImGui::Selectable((displayName + "##" + renderer->GetUUID().Get()).c_str(), isSelected))
						{
							selectedRenderer = renderer;
							selectedEntity = entity;
						}
						index++;
					}
					
				}
				ImGui::EndCombo();
			}

			ImGui::SameLine();
			if (ImGui::Button("Add") && selectedRenderer)
			{
				animator->AddMeshRenderer(selectedRenderer);
				selectedRenderer = nullptr;
			}

			if (animator->GetOwner()->HasComponent<MeshRenderer>()) {
				if (ImGui::Button("Get Self"))
				{
					std::vector<MeshRenderer*> renderersInEntity = animator->GetOwner()->GetComponents<MeshRenderer>();
					for(const auto renderer : renderersInEntity)
						animator->AddMeshRenderer(renderer);
				}
				ImGui::SameLine();
			}

			if (ImGui::Button("Add All Childs"))
			{
				std::shared_ptr<Entity> owner = animator->GetOwner();

				std::function<void(Entity*)> addFrom = [&](Entity* entity) {
					std::vector<MeshRenderer*> renderersInEntity = entity->GetComponents<MeshRenderer>();
					for (const auto renderer : renderersInEntity)
						animator->AddMeshRenderer(renderer);

					for (auto& child : entity->GetChildren())
						addFrom(child.get());
				};

				addFrom(owner.get());
			}
			ImGui::SameLine();
			if (ImGui::Button("Clear"))
			{
				animator->ClearRenderers();
			}

			static std::unordered_map<Animator*, std::string> s_selectedTargetUUID;

			if (!renderers.empty())
			{
				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Text("Configuration");
				
				MeshRenderer* targetRenderer = animator->GetTargetRenderer();

				if (!targetRenderer)
				{
					targetRenderer = animator->GetFirstMeshRenderer();
					if (targetRenderer)
					{
						ImGui::TextColored(ImVec4(1, 1, 0, 1), "No target renderer selected. Using first.");
						ImGui::SameLine();
						if (ImGui::SmallButton("Set as Target"))
						{
							animator->SetTargetRenderer(targetRenderer);
						}
					}
				}

				if (targetRenderer)
				{
					std::shared_ptr<Entity> owner = targetRenderer->GetOwner();
					auto allRenderersInEntity = owner->GetComponents<MeshRenderer>();

					std::string targetLabel = owner->GetName();
					if (allRenderersInEntity.size() > 1) {
						int idx = 0;
						for (int i = 0; i < allRenderersInEntity.size(); i++) {
							if (allRenderersInEntity[i] == targetRenderer) {
								idx = i;
								break;
							}
						}
						targetLabel += fmt::format(" [{}]", idx);
					}
					ImGui::Text("Active Target: %s", targetLabel.c_str());

					if (ImGui::BeginCombo("Switch Target", "Select..."))
					{
						for (const auto& [uuid, data] : renderers)
						{
							MeshRenderer* renderer = data.Renderer;
							if (!renderer || renderer == targetRenderer) 
								continue;

							std::shared_ptr<Entity> rOwner = renderer->GetOwner();
							auto rRenderers = rOwner->GetComponents<MeshRenderer>();

							std::string displayName = rOwner->GetName();
							if (rRenderers.size() > 1) {
								int idx = 0;
								for (int i = 0; i < rRenderers.size(); i++) {
									if (rRenderers[i] == renderer) {
										idx = i;
										break;
									}
								}
								displayName += fmt::format(" [{}]", idx);
							}

							if (ImGui::Selectable(displayName.c_str()))
							{
								animator->SetTargetRenderer(renderer);
							}
						}
						ImGui::EndCombo();
					}

					std::shared_ptr<Mesh> mesh = targetRenderer->GetMesh();
					if (mesh)
					{
						auto& clips = mesh->GetData().AnimationClips;
						int totalClips = static_cast<int>(clips.size());

						if (totalClips > 0)
						{
							int selectedClipIndex = animator->GetCurrentClipIndex();
							if (selectedClipIndex >= totalClips)
								selectedClipIndex = 0;

							const char* currentClipName = clips[selectedClipIndex].Name.c_str();

							if (ImGui::BeginCombo("Animation Clip", currentClipName))
							{
								for (int i = 0; i < totalClips; i++)
								{
									bool isSelected = (selectedClipIndex == i);
									if (ImGui::Selectable(clips[i].Name.c_str(), isSelected))
									{
										selectedClipIndex = i;
										animator->SelectClip(clips[i].Name);
									}
									if (isSelected)
										ImGui::SetItemDefaultFocus();
								}
								ImGui::EndCombo();
							}

							if (ImGui::Button("Play")) animator->Play();
							ImGui::SameLine();
							if (ImGui::Button("Pause")) animator->Pause();
							ImGui::SameLine();
							if (ImGui::Button("Resume")) animator->Resume();
							ImGui::SameLine();
							if (ImGui::Button("Stop")) animator->Stop();
						}
						else
							ImGui::Text("No animation clips in target mesh.");
					}
					else
						ImGui::Text("Target renderer has no mesh.");
				}
			}
			else
				ImGui::Text("Add a MeshRenderer to enable animation controls.");
		}

		ImGui::PopID();
	}

	void InspectorInterface::DrawScriptClass(ScriptClass* scriptClass)
	{
		ImGui::PushID(scriptClass);

		bool open = ImGui::CollapsingHeader(scriptClass->GetClassName().c_str());
		ImGui::SetItemTooltip(scriptClass->GetUUID().Get().c_str());
		if (ComponentContextMenu(scriptClass)) {
			ImGui::PopID();
			return;
		}

		bool isRuntime = ScriptingManager::IsRunning();

		if (open) {
			/// Get Fields and show (one runtime version, and one editor version) -> For now, this is only editor version

			std::shared_ptr<ScriptingClass> scriptingClass = ScriptingManager::s_Data.ScriptingClasses[scriptClass->GetClassName()];
			if (!scriptingClass)
			{
				ImGui::Text("Script ERROR (Not Found)");
				ImGui::PopID();
				return;
			}
			const std::map<std::string, ScriptField>& fields = scriptingClass->GetFields();
			for (const auto& [name, field] : fields)
			{
				switch (field.Type)
				{
				case ScriptFieldType::Float:
				{
					float v = isRuntime ? scriptClass->GetRuntimeFieldValue<float>(name) : scriptClass->GetFieldValue<float>(name);
					if (ImGui::DragFloat(name.c_str(), &v, 0.1f))
						isRuntime ? scriptClass->SetRuntimeFieldValue(name, v) : scriptClass->SetFieldValue(name, v);
					break;
				}
				case ScriptFieldType::Double:
				{
					double v = isRuntime ? scriptClass->GetRuntimeFieldValue<double>(name) : scriptClass->GetFieldValue<double>(name);
					if (ImGui::DragScalar(name.c_str(), ImGuiDataType_Double, &v, 0.1f))
						isRuntime ? scriptClass->SetRuntimeFieldValue(name, v) : scriptClass->SetFieldValue(name, v);
					break;
				}
				case ScriptFieldType::Bool:
				{
					bool v = isRuntime ? scriptClass->GetRuntimeFieldValue<bool>(name) : scriptClass->GetFieldValue<bool>(name);
					if (ImGui::Checkbox(name.c_str(), &v))
						isRuntime ? scriptClass->SetRuntimeFieldValue(name, v) : scriptClass->SetFieldValue(name, v);
					break;
				}
				case ScriptFieldType::Char:
				{
					int v = isRuntime ? scriptClass->GetRuntimeFieldValue<char>(name) : scriptClass->GetFieldValue<char>(name);
					if (ImGui::DragInt(name.c_str(), &v, 1, 0, 255))
						isRuntime ? scriptClass->SetRuntimeFieldValue(name, (char)v) : scriptClass->SetFieldValue(name, (char)v);
					break;
				}

				case ScriptFieldType::Byte:
				{
					int v = isRuntime ? scriptClass->GetRuntimeFieldValue<uint8_t>(name) : scriptClass->GetFieldValue<uint8_t>(name);
					if (ImGui::DragInt(name.c_str(), &v, 1, 0, 255))
						isRuntime ? scriptClass->SetRuntimeFieldValue(name, (uint8_t)v) : scriptClass->SetFieldValue(name, (uint8_t)v);
					break;
				}

				case ScriptFieldType::Short:
				case ScriptFieldType::UShort:
				case ScriptFieldType::Int:
				case ScriptFieldType::UInt:
				{
					int v = isRuntime ? scriptClass->GetRuntimeFieldValue<int>(name) : scriptClass->GetFieldValue<int>(name);
					if (ImGui::DragInt(name.c_str(), &v))
						isRuntime ? scriptClass->SetRuntimeFieldValue(name, v) : scriptClass->SetFieldValue(name, v);
					break;
				}

				case ScriptFieldType::Long:
				case ScriptFieldType::ULong:
				{
					int64_t v = isRuntime ? scriptClass->GetRuntimeFieldValue<int64_t>(name) : scriptClass->GetFieldValue<int64_t>(name);
					if (ImGui::DragScalar(name.c_str(), ImGuiDataType_S64, &v))
						isRuntime ? scriptClass->SetRuntimeFieldValue(name, v) : scriptClass->SetFieldValue(name, v);
					break;
				}

				case ScriptFieldType::String:
				{
					std::string value = isRuntime ? scriptClass->GetRuntimeFieldString(name) :  scriptClass->GetFieldString(name);

					char buffer[256];
					memset(buffer, 0, sizeof(buffer));
					strncpy(buffer, value.c_str(), sizeof(buffer) - 1);

					if (ImGui::InputText(name.c_str(), buffer, sizeof(buffer)))
						isRuntime ? scriptClass->SetRuntimeFieldString(name, std::string(buffer)) : scriptClass->SetFieldString(name, std::string(buffer));

					break;
				}

				default:
					ImGui::Text("Unsupported Field: %s", name.c_str());
					break;
				}
			}
		}
		ImGui::PopID();
	}

	void InspectorInterface::DrawCanvas(Canvas* canvas)
	{
		ImGui::PushID(canvas);
		bool open = ImGui::CollapsingHeader("Canvas");
		ImGui::SetItemTooltip(canvas->GetUUID().Get().c_str());
		if (ComponentContextMenu(canvas)) {
			ImGui::PopID();
			return;
		}
		if (open) {
			CanvasRenderMode renderMode = canvas->GetRenderMode();
			int modeIndex = (int)renderMode;
			const char* modeLabels[] = { "World", "Overlay" };
			if (ImGui::Combo("Render Mode", &modeIndex, modeLabels, IM_ARRAYSIZE(modeLabels))) {
				canvas->SetRenderMode((CanvasRenderMode)modeIndex);
			}
			if (canvas->GetRenderMode() == CanvasRenderMode::ScreenSpaceOverlay)
			{
				//.............
			}
		}
		ImGui::PopID();
	}

	void InspectorInterface::DrawImage(Image* image)
	{
		if (!image)
			return;

		ImGui::PushID(image);

		bool open = ImGui::CollapsingHeader("Image");
		ImGui::SetItemTooltip(image->GetUUID().Get().c_str());
		if (ComponentContextMenu(image))
		{
			ImGui::PopID();
			return;
		}

		if (open)
		{
			vec4 color = image->GetTint();
			ImVec4 imColor(color.r, color.g, color.b, color.a);

			ImGui::Text("Tint");
			ImGui::SameLine();

			if (ImGui::ColorEdit4("##ImageTint", (float*)&imColor))
			{
				image->SetTint(vec4(imColor.x, imColor.y, imColor.z, imColor.w));
			}
			ImGui::Separator();

			std::shared_ptr<Texture> texture = image->GetTexture();
			Metadata* meta = texture ? AssetRegistry::GetMetadata(texture->GetUUID()) : nullptr;

			ImGui::Text("Texture: %s", meta ? "Assigned" : "None / Unknown");

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_EXPLORER_FILE"))
				{
					const char* droppedPath = (const char*)payload->Data;
					if (droppedPath && TextureImporter::CheckIfIsImage(droppedPath))
					{
						Metadata& droppedMeta = AssetRegistry::GetOrCreateMetadata(droppedPath);
						TextureImporter::ImportImage(droppedPath, droppedMeta);

						std::shared_ptr<Texture> droppedTex = ResourceManager::GetTexture(droppedMeta);
						if (droppedTex)
							droppedTex->Load();

						image->SetTexture(droppedTex);
					}
				}
				ImGui::EndDragDropTarget();
			}

			texture = image->GetTexture();
			if (texture)
			{
				meta = AssetRegistry::GetMetadata(texture->GetUUID());

				if (meta && !meta->CachesPath.empty())
					ImGui::Text("Path: %s", meta->CachesPath[0].c_str());

				ivec2 texSize = texture->GetSize();
				ImGui::Text("Size: %d x %d", texSize.x, texSize.y);

				ImGui::Image((ImTextureID)texture->GetRendererId(), ImVec2(64, 64), ImVec2(1, 0), ImVec2(0, 1));

				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_EXPLORER_FILE"))
					{
						const char* droppedPath = (const char*)payload->Data;
						if (droppedPath && TextureImporter::CheckIfIsImage(droppedPath))
						{
							Metadata& droppedMeta = AssetRegistry::GetOrCreateMetadata(droppedPath);
							TextureImporter::ImportImage(droppedPath, droppedMeta);

							std::shared_ptr<Texture> droppedTex = ResourceManager::GetTexture(droppedMeta);
							if (droppedTex)
								droppedTex->Load();

							image->SetTexture(droppedTex);
						}
					}
					ImGui::EndDragDropTarget();
				}
			}

			ImGui::TextDisabled("Drag & drop an image from Assets Explorer onto the texture field/preview.");
		}

		ImGui::PopID();
	}

	void InspectorInterface::DrawBoxCollider(BoxCollider* boxCollider) 
	{

		ImGui::PushID(boxCollider);
		bool open = ImGui::CollapsingHeader("Box Collider");
		ImGui::SetItemTooltip(boxCollider->GetUUID().Get().c_str());
		if (ComponentContextMenu(boxCollider)) {
			ImGui::PopID();
			return;
		}
		if (open) {
			vec3 center = boxCollider->GetLocalCenter();
			vec3 extents = boxCollider->GetLocalExtents();
			bool draw = boxCollider->GetDrawGizmo();

			if (ImGui::DragFloat3("Center", &center.x, 0.01f))
				boxCollider->SetLocalCenter(center);

			if (ImGui::DragFloat3("Extents", &extents.x, 0.01f))
				boxCollider->SetLocalExtents(extents);

			//if (ImGui::Checkbox("Visible Lines", &draw))
			//	boxCollider->SetDrawGizmo(draw);
		}
		ImGui::PopID();
	}

	void InspectorInterface::AddComponent(const std::shared_ptr<Entity>& entity)
	{
		if (!entity)
			return;

		ImGui::Separator();

		static ImGuiTextFilter filter;
		static bool isOpen = false;

		bool forceClose = false;



		ImGui::SetNextWindowSizeConstraints(ImVec2(350, 300),ImVec2(350, 600));

		if (ImGui::BeginCombo("##AddComponentCombo", "Add Component..."))
		{
			ImGui::Text("Search");
			ImGui::SameLine();
			filter.Draw("Search", -1.0f);
			ImGui::Separator();

			ImVec4 windowBg = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
			ImGui::PushStyleColor(ImGuiCol_ChildBg, windowBg);
			ImGui::BeginChild("ComponentScrollRegion", ImVec2(0, 0), false);

			bool searching = strlen(filter.InputBuf) > 0;

			if (searching)
				ImGui::SetNextItemOpen(true, ImGuiCond_Always);
			if(!isOpen)
				ImGui::SetNextItemOpen(false, ImGuiCond_Always);
			if (ImGui::CollapsingHeader("General"))
			{
				ImGui::Indent(8.0f);


				if (filter.PassFilter("Animator"))
				{
					if (ImGui::Selectable("Animator"))
					{
						entity->AddComponent<Animator>();
						forceClose = true;
					}
				}

				if (!entity->HasComponent<Camera>() && filter.PassFilter("Camera"))
				{
					if (ImGui::Selectable("Camera"))
					{
						entity->AddComponent<Camera>();
						forceClose = true;
					}
				}

				if (filter.PassFilter("Mesh Renderer"))
				{
					if (ImGui::Selectable("Mesh Renderer"))
					{
						entity->AddComponent<MeshRenderer>();
						forceClose = true;
					}
				}

				ImGui::Unindent(8.0f);
			}



			if (searching)
				ImGui::SetNextItemOpen(true, ImGuiCond_Always);
			if (!isOpen)
				ImGui::SetNextItemOpen(false, ImGuiCond_Always);
			if (ImGui::CollapsingHeader("Collisions"))
			{
				ImGui::Indent(8.0f);

				if (filter.PassFilter("Box Collider")) {
					if (ImGui::Selectable("Box Collider")) {
						entity->AddComponent<BoxCollider>();
						forceClose = true;
					}
				}

				ImGui::Unindent(8.0f);
			}



			if (searching)
				ImGui::SetNextItemOpen(true, ImGuiCond_Always);
			if (!isOpen)
				ImGui::SetNextItemOpen(false, ImGuiCond_Always);
			if (ImGui::CollapsingHeader("UI"))
			{
				ImGui::Indent(8.0f);
				if (filter.PassFilter("Canvas"))
				{
					if (ImGui::Selectable("Canvas"))
					{
						entity->AddComponent<Canvas>();
						forceClose = true;
					}
				}

				if (filter.PassFilter("Image"))
				{
					if (ImGui::Selectable("Image"))
					{
						entity->AddComponent<Image>();
						forceClose = true;
					}
				}
				ImGui::Unindent(8.0f);
			}



			if (searching)
				ImGui::SetNextItemOpen(true, ImGuiCond_Always);
			if (!isOpen)
				ImGui::SetNextItemOpen(false, ImGuiCond_Always);
			if (ImGui::CollapsingHeader("Scripts"))
			{
				ImGui::Indent(8.0f);
				const auto& scriptingClasses = ScriptingManager::s_Data.ScriptingClasses;

				for (const auto& scriptClass : scriptingClasses)
				{
					if (!scriptClass.second)
						continue;

					const std::string& className = scriptClass.second->GetClassName();

					if (!filter.PassFilter(className.c_str()))
						continue;

					if (ImGui::Selectable(className.c_str()))
					{
						entity->AddComponent<ScriptClass>(scriptClass.first);
						forceClose = true;
					}
				}
				ImGui::Unindent(8.0f);
			}

			if (forceClose) {
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndChild();
			ImGui::PopStyleColor();
			ImGui::EndCombo();	

			isOpen = true;
		}
		else
		{
			filter.InputBuf[0] = '\0';
			isOpen = false;
		}
	}

	void InspectorInterface::DrawMaterial(std::shared_ptr<Material> material)
	{
		bool isEditable = material->IsEditable();

		std::string materialName = "Material";
		if (!isEditable)
			materialName += " (Read-Only -> EngineDefault)";
		ImGui::Text(materialName.c_str());
		ImGui::Text("Material Resource Count: %u", material->GetReferenceCount());
		const std::unordered_map<std::string, UniformValue> properties = material->GetUniforms();

		std::shared_ptr<Texture> texture = material->GetTexture();
		if (texture) {
			Metadata* metadata = AssetRegistry::GetMetadata(material->GetTexture()->GetUUID());
			ImGui::Text("Path: %s", metadata->CachesPath[0].c_str());
			ImGui::Text("Texture Resource Count: %u", material->GetTexture()->GetReferenceCount());
			ivec2 texSize = material->GetTexture()->GetSize();
			ImGui::Text("Size: %d x %d", texSize.x, texSize.y);
			ImGui::Separator();
		}



		if (!isEditable)
			ImGui::BeginDisabled();

		for (auto& [name, uniform] : properties)
		{

			switch (uniform.type)
			{
			case UniformType_int:
			{
				int value = std::get<int>(uniform.value);

				ImGui::Text("%s", name.c_str());
				ImGui::SameLine();

				if (ImGui::DragInt(("##" + name).c_str(), &value))
				{
					UniformValue newVal = uniform;
					newVal.value = value;
					material->SetShaderVariable(name, newVal);
				}
				break;
			}
			case UniformType_uint:
			{
				unsigned int value = std::get<unsigned int>(uniform.value);

				ImGui::Text("%s", name.c_str());
				ImGui::SameLine();

				ImGui::SetNextItemWidth(100);
				if (ImGui::DragScalar(("##" + name).c_str(), ImGuiDataType_U32, &value, 1.0f))
				{
					UniformValue newVal = uniform;
					newVal.value = value;
					material->SetShaderVariable(name, newVal);
				}
				break;
			}
			case UniformType_float:
			{
				float value = std::get<float>(uniform.value);

				ImGui::Text("%s", name.c_str());
				ImGui::SameLine();

				ImGui::SetNextItemWidth(100);
				if (ImGui::DragFloat(("##" + name).c_str(), &value, 0.01f))
				{
					UniformValue newVal = uniform;
					newVal.value = value;
					material->SetShaderVariable(name, newVal);
				}
				break;
			}
			case UniformType_bool:
			{
				bool value = std::get<bool>(uniform.value);

				ImGui::Text("%s", name.c_str());
				ImGui::SameLine();

				if (ImGui::Checkbox(("##" + name).c_str(), &value))
				{
					UniformValue newVal = uniform;
					newVal.value = value;
					material->SetShaderVariable(name, newVal);
				}
				break;
			}
			case UniformType_vec2:
			{
				vec2 value = std::get<vec2>(uniform.value);

				ImGui::Text("%s", name.c_str());
				ImGui::SameLine();

				if (ImGui::DragFloat2(("##" + name).c_str(), &value.x, 0.01f))
				{
					UniformValue newVal = uniform;
					newVal.value = value;
					material->SetShaderVariable(name, newVal);
				}
				break;
			}
			case UniformType_vec3:
			{
				vec3 value = std::get<vec3>(uniform.value);

				ImGui::Text("%s", name.c_str());
				ImGui::SameLine();

				if (ImGui::DragFloat3(("##" + name).c_str(), &value.x))
				{
					UniformValue newVal = uniform;
					newVal.value = value;
					material->SetShaderVariable(name, newVal);
				}
				break;
			}
			case UniformType_vec4:
			{
				vec4 value = std::get<vec4>(uniform.value);
				ImVec4 color(value.x, value.y, value.z, value.w);

				ImGui::Text("%s", name.c_str());
				ImGui::SameLine();

				if (ImGui::ColorButton(("##btn_" + name).c_str(), color, ImGuiColorEditFlags_NoTooltip, ImVec2(100, 0)))
					ImGui::OpenPopup(("picker_" + name).c_str());

				if (ImGui::BeginPopup(("picker_" + name).c_str()))
				{
					ImGui::Text("Select Color");
					if (ImGui::ColorPicker4(("##picker_" + name).c_str(), (float*)&color, ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_AlphaBar))
					{
						vec4 newValue = vec4(color.x, color.y, color.z, color.w);

						UniformValue newVal = uniform;
						newVal.value = newValue;
						material->SetShaderVariable(name, newVal);
					}
					ImGui::EndPopup();
				}
				break;
			}
			case UniformType_Sampler2D:

				break;
			default:
				break;
			}
		}

		if (!isEditable)
			ImGui::EndDisabled();
		else {
			if (ImGui::Button("Apply")) {
				material->Save();
			}
		}
	}

	void InspectorInterface::OnNotify(const OnEntityOrFileNotification& id)
	{
		if (id == OnEntityOrFileNotification::OnEntitySelect) {
			m_mode = InspectorMode::EntityMode;
		}
		else if (id == OnEntityOrFileNotification::OnFileSelect) {
			m_mode = InspectorMode::ImportMode;
		}
	}

	bool InspectorInterface::ComponentContextMenu(Component* component, bool canRemove)
	{
		if (ImGui::BeginPopupContextItem("Component Options"))
		{
			if (ImGui::MenuItem("Copy UUID"))
			{
				Application::GetInstance().m_clipboard.Copy(component->GetUUID().Get());
				Log::Warn("{0}", Application::GetInstance().m_clipboard.Paste());
			}
			if (canRemove && ImGui::MenuItem("Remove Component"))
			{
				component->GetOwner()->RemoveComponent(component);
				ImGui::EndPopup();
				return true;
			}
			ImGui::EndPopup();
		}
		return false;
	}
}