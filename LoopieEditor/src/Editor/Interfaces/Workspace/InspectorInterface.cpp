#include "InspectorInterface.h"
#include "Editor/Interfaces/Workspace/HierarchyInterface.h"
#include "Editor/Interfaces/Workspace/AssetsExplorerInterface.h"

#include "Loopie/Core/Log.h"
#include "Loopie/Core/Application.h"
#include "Loopie/Math/MathTypes.h"

#include "Loopie/Components/Transform.h"
#include "Loopie/Components/Camera.h"
#include "Loopie/Components/MeshRenderer.h"
#include "Loopie/Components/ScriptClass.h"
#include "Loopie/Components/Animator.h"

#include "Loopie/Scripting/ScriptingManager.h"

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
				DrawFileImportSettings(AssetsExplorerInterface::s_SelectedFile);
				break;
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

		bool isActive = entity->GetIsActive();
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

		bool open = ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen);
		bool modified = false;
		
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
		}
		ImGui::PopID();

		if(modified)
			Application::GetInstance().GetScene().GetOctree().Rebuild();
	}

	void InspectorInterface::DrawCamera(Camera* camera)
	{
		ImGui::PushID(camera);

		bool open = ImGui::CollapsingHeader("Camera");

		if (ComponentContextMenu(camera)) {
			ImGui::PopID();
			return;
		}

		if (open) {
			float fov = camera->GetFov();
			float nearPlane = camera->GetNearPlane();
			float farPlane = camera->GetFarPlane();
			bool isMainCamera = Camera::GetMainCamera() == camera;

			if (ImGui::DragFloat("Fov", &fov, 1.0f, 1.0f, 179.0f))
				camera->SetFov(fov);

			if (ImGui::DragFloat("Near Plane", &nearPlane, 0.01f, 0.01f, farPlane - 0.01f))
				camera->SetNearPlane(nearPlane);

			if (ImGui::DragFloat("Far Plane", &farPlane, 1.0f, nearPlane + 0.1f, 10000.0f))
				camera->SetFarPlane(farPlane);

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
					int maxIndex = data->CachesPath.size();
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
						int maxIndex = data->CachesPath.size();
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
					for (size_t i = 0; i < allRenderersInEntity.size(); i++) {
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
						for (size_t i = 0; i < allRenderersInEntity.size(); i++) {
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
								for (size_t i = 0; i < rRenderers.size(); i++) {
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

		if (ComponentContextMenu(scriptClass)) {
			ImGui::PopID();
			return;
		}

		bool isRuntime = ScriptingManager::IsRunning();

		if (open) {
			/// Get Fields and show (one runtime version, and one editor version) -> For now, this is only editor version

			std::shared_ptr<ScriptingClass> scriptingClass = ScriptingManager::s_Data.ScriptingClasses[scriptClass->GetClassName()];
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
					if (ImGui::DragScalar(name.c_str(), ImGuiDataType_Double, &v, 0.1))
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

	void InspectorInterface::AddComponent(const std::shared_ptr<Entity>& entity)
	{
		if (!entity)
			return;

		ImGui::Separator();

		static const char* previewLabel = "Add Component...";
		static int selectedIndex = -1;

		if (ImGui::BeginCombo("##AddComponentCombo", previewLabel))
		{
			if (!entity->HasComponent<Camera>())
			{
				if (ImGui::Selectable("Camera"))
				{
					entity->AddComponent<Camera>();
					ImGui::EndCombo();
					return;
				}
			}

			if (ImGui::Selectable("Mesh Renderer"))
			{
				entity->AddComponent<MeshRenderer>();
				ImGui::EndCombo();
				return;
			}

			if (ImGui::Selectable("Animator"))
			{
				entity->AddComponent<Animator>();
				ImGui::EndCombo();
				return;
			}


			///// How To Add More Components
			// 
			//if (ImGui::Selectable(""))
			//{
			//    entity->AddComponent<>();
			//    ImGui::EndCombo();
			//    return;
			//}

			const std::unordered_map<std::string, std::shared_ptr<ScriptingClass>>& scriptingClasses = ScriptingManager::s_Data.ScriptingClasses;
			for(const auto& scriptClass : scriptingClasses)
			{
				if (ImGui::Selectable(scriptClass.second->GetClassName().c_str()))
				{
					entity->AddComponent<ScriptClass>(scriptClass.first);
					ImGui::EndCombo();
					return;
				}
			}

			

			ImGui::EndCombo();
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