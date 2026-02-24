#include "InspectorInterface.h"
#include "Editor/Interfaces/Workspace/HierarchyInterface.h"
#include "Editor/Interfaces/Workspace/AssetsExplorerInterface.h"

#include "Loopie/Core/Log.h"
#include "Loopie/Math/MathTypes.h"

#include "Loopie/Components/Transform.h"
#include "Loopie/Components/Camera.h"
#include "Loopie/Components/MeshRenderer.h"
#include "Loopie/Components/ScriptClass.h"

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
		
		ImGui::Separator();
		
		ImGui::TextDisabled("UUID: %s", entity->GetUUID().Get().c_str());

		ImGui::Separator();
	}

	void InspectorInterface::DrawTransform(Transform* transform)
	{
		ImGui::PushID(transform);

		bool open = ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen);
		bool modified = false;
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

		if (RemoveComponent(camera)) {
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

		if (RemoveComponent(meshRenderer)) {
			ImGui::PopID();
			return;
		}

		if (open) {
			auto mesh = meshRenderer->GetMesh();
			ImGui::Text("Mesh: %s", mesh ? "Assigned" : "None");
			if (!mesh) {
				ImGui::PopID();
				return;
			}
			ImGui::Text("Mesh Resource Count: %u", mesh->GetReferenceCount());
			ImGui::Text("Mesh Vertices: %d", mesh->GetData().VerticesAmount);

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

		RemoveComponent(meshRenderer);	
		ImGui::PopID();
	}

	void InspectorInterface::DrawScriptClass(ScriptClass* scriptClass)
	{
		ImGui::PushID(scriptClass);

		bool open = ImGui::CollapsingHeader(scriptClass->GetClassName().c_str());

		if (RemoveComponent(scriptClass)) {
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

	bool InspectorInterface::RemoveComponent(Component* component)
	{
		if (!component)
			return false;

		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Remove Component"))
			{
				component->GetOwner()->RemoveComponent(component);
				ImGui::EndPopup();
				return true;
			}
			ImGui::EndPopup();
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
}