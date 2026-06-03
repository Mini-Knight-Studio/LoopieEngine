#include "Renderer.h"

#include "Loopie/Core/Assert.h"
#include "Loopie/Core/Time.h"
#include "Loopie/Project/Project.h"
#include "Loopie/Project/ProjectConfig.h"
#include "Loopie/Components/Transform.h"
#include "Loopie/Render/Gizmo.h"
#include "Loopie/Render/ShadowMap.h"
#include <iostream>

#include <glad/glad.h>
#include <IL/il.h>
#include <IL/ilu.h>

namespace Loopie {
	std::vector<Renderer::RenderItem> Renderer::s_OpaqueRenderQueue = std::vector<Renderer::RenderItem>();
	std::vector<Renderer::RenderItem> Renderer::s_TransparentRenderQueue = std::vector<Renderer::RenderItem>();
	std::vector<Camera*> Renderer::s_RenderCameras = std::vector<Camera*>();
	std::shared_ptr<UniformBuffer> Renderer::s_MatricesUniformBuffer = nullptr;
	std::shared_ptr<UniformBuffer> Renderer::s_LightingUniformBuffer = nullptr;
	std::shared_ptr<UniformBuffer> Renderer::s_ShadowingUniformBuffer = nullptr;
	std::shared_ptr<UniformBuffer> Renderer::s_StaticMatricesUniformBuffer = nullptr;
	std::shared_ptr<ShaderStorageBuffer> Renderer::s_BonesSSBO = nullptr;
	unsigned int Renderer::s_BoneBufferOffset = 0;
	unsigned int Renderer::s_BoneBufferCapacity = 0;

	Renderer::ParticlesData Renderer::s_ParticlesData;
	std::vector<Renderer::ParticleSortData> Renderer::s_ParticleQueue;

	std::unordered_set<Shader*> Renderer::s_FrameUpdatedShaders;

	vec4 Renderer::s_CurrentViewport = {0,0,0,0};

	ShadowSettings Renderer::s_ShadowSettings = ShadowSettings();
	ShadowQuality Renderer::s_PendingShadowQuality = ShadowQuality::Medium;
	ShadowFilter Renderer::s_PendingShadowFilter = ShadowFilter::Soft;

	bool Renderer::s_UseGizmos = true;
	Light* Renderer::s_Lights[MAX_LIGHTS] = {};
	unsigned short Renderer::s_LightCount = 0;
	Renderer::ShadowSlot Renderer::s_ShadowSlots[MAX_SHADOW_CASTING_LIGHTS] = { nullptr };
	unsigned short Renderer::s_ShadowCount = 0;
	std::unique_ptr<Shader> Renderer::s_ShadowMapShader = nullptr;
	std::unique_ptr<Shader> Renderer::s_TonemapShader = nullptr;
	GLuint Renderer::s_DummyVAO = 0;
	float Renderer::s_Exposure = 1.0f;
	std::vector<std::shared_ptr<FrameBuffer>> Renderer::s_BloomChain = { nullptr };
	std::unique_ptr<Shader> Renderer::s_BloomExtractShader = nullptr;
	float Renderer::s_BloomThreshold = 1.0f;
	float Renderer::s_BloomStrength = 0.04f;
	std::unique_ptr<Shader> Renderer::s_BloomDownsampleShader = nullptr;
	std::unique_ptr<Shader> Renderer::s_BloomUpsampleShader = nullptr;

	unsigned int Renderer::s_SceneDepthTextureID = 0;
	float Renderer::s_NearPlane = 0.3f;
	float Renderer::s_FarPlane = 200.0f;


	static bool CompareByMaterial(const Renderer::RenderItem& a, const Renderer::RenderItem& b)
	{
		if (a.Material->GetShader().GetProgramID() != b.Material->GetShader().GetProgramID())
			return a.Material->GetShader().GetProgramID() < b.Material->GetShader().GetProgramID();

		return a.Material.get() < b.Material.get();
	}

	void Renderer::Init(void* context) {
		ASSERT(!gladLoadGLLoader((GLADloadproc)context), "Failed to Initialize GLAD!");

		glEnable(GL_BLEND);
		EnableDepth();
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// DevIL Init
		ilInit();
		iluInit();

		ilSetInteger(IL_KEEP_DXTC_DATA, IL_FALSE);
		ilSetInteger(IL_ORIGIN_MODE, IL_ORIGIN_LOWER_LEFT);

		// Gizmo Data Structure Init
		Gizmo::Init();

		BufferLayout layout;
		layout.AddLayoutElement(0, GLVariableType::MATRIX4, 1, "View");
		layout.AddLayoutElement(1, GLVariableType::MATRIX4, 1, "Proj");
		s_MatricesUniformBuffer = std::make_shared<UniformBuffer>(layout);
		s_MatricesUniformBuffer->BindToLayout(0);

		BufferLayout lightingLayout;
		lightingLayout.AddLayoutElement(0, GLVariableType::VEC4, 1, "CameraWorldPosLightCount"); // Camera World Position + Light Count
		for (int i = 0; i < MAX_LIGHTS; ++i)
		{
			lightingLayout.AddLayoutElement(5*i+1, GLVariableType::VEC4, 1, "ColorIntensity"); // Color + Intensity
			lightingLayout.AddLayoutElement(5*i+2, GLVariableType::VEC4, 1, "PositionType"); // Position + Type
			lightingLayout.AddLayoutElement(5*i+3, GLVariableType::VEC4, 1, "DirectionInnerCone"); // Direction + Inner Cone Angle
			lightingLayout.AddLayoutElement(5*i+4, GLVariableType::VEC4, 1, "AttenuationOuterCone"); // Attenuation + Outer Cone Angle
			lightingLayout.AddLayoutElement(5*i+5, GLVariableType::VEC4, 1, "SMapAndSColor"); // ShadowMap number + Shadow Color
		}
		s_LightingUniformBuffer = std::make_shared<UniformBuffer>(lightingLayout);
		s_LightingUniformBuffer->BindToLayout(1);

		
		s_BonesSSBO = std::make_shared<ShaderStorageBuffer>();


		s_BoneBufferCapacity = MAX_BONES_TOTAL * sizeof(matrix4);

		s_BonesSSBO->Bind();
		s_BonesSSBO->SetData(nullptr, s_BoneBufferCapacity);

		s_BonesSSBO->BindToLayout(2);


		s_ParticlesData.TransformVBO = std::make_shared<VertexBuffer>(nullptr, MAX_PARTICLES * sizeof(matrix4));
		s_ParticlesData.ColorVBO = std::make_shared<VertexBuffer>(nullptr, MAX_PARTICLES * sizeof(vec4));
		s_ParticlesData.transformsBatch.reserve(MAX_PARTICLES);
		s_ParticlesData.colorsBatch.reserve(MAX_PARTICLES);

		s_OpaqueRenderQueue.reserve(1000);
		s_TransparentRenderQueue.reserve(300);
		s_ParticleQueue.reserve(3000);
		s_FrameUpdatedShaders.reserve(30);
		

		s_LightCount = 0;
		InitShadowMapping();
		InitPostProcessing();
	}

	void Renderer::Shutdown() {
		ilShutDown();
		Gizmo::Shutdown();
	}

	void Renderer::Clear() {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}

	void Renderer::SetClearColor(const vec4& color) {
		glClearColor(color.x, color.y, color.z, color.w);
	}

	void Renderer::SetViewport(unsigned int x, unsigned int y, unsigned int width, unsigned int height)
	{
		s_CurrentViewport = vec4(x, y, width, height);
		glViewport(x, y, width, height);
	}

	void Renderer::RegisterLight(Light* light)
	{
		if (s_LightCount >= MAX_LIGHTS)
		{
			Log::Warn("Exceeded maximum amount of lights. Cannot register light.");
			return;
		}

		s_Lights[s_LightCount] = light;
		s_LightCount++;
	}

	void Renderer::UnregisterLight(Light* light)
	{
		for (int i = 0; i < s_LightCount; ++i)
		{
			if (s_Lights[i] == light)
			{
				s_Lights[i] = s_Lights[s_LightCount - 1];
				s_LightCount--;
				return;
			}
		}
		Log::Warn("UnregisterLight: Tried to unregister a non-registered light - Light not found.");
	}

	void Renderer::RemoveAllLights()
	{
		s_LightCount = 0;
	}

	void Renderer::InitShadowMapping()
	{
		s_ShadowMapShader = std::make_unique<Shader>("assets/shaders/ShadowDepth.shader");
		if (!s_ShadowMapShader->GetIsValidShader())
		{
			Log::Error("Shadow depth shader failed to compile!");
		}

		BufferLayout dynamicShadowingLayout;
		for (int i = 0; i < MAX_SHADOW_CASTING_LIGHTS; ++i)
		{
			dynamicShadowingLayout.AddLayoutElement(i, GLVariableType::MATRIX4, 1, "DynamicLightSpaceMatrix");
		}
		s_ShadowingUniformBuffer = std::make_shared<UniformBuffer>(dynamicShadowingLayout);
		s_ShadowingUniformBuffer->BindToLayout(3);

		BufferLayout staticShadowingLayout;
		for (int i = 0; i < MAX_SHADOW_CASTING_LIGHTS; ++i)
		{
			staticShadowingLayout.AddLayoutElement(i, GLVariableType::MATRIX4, 1, "StaticLightSpaceMatrix");
		}
		s_StaticMatricesUniformBuffer = std::make_shared<UniformBuffer>(staticShadowingLayout);
		s_StaticMatricesUniformBuffer->BindToLayout(4);

		BuildShadowMaps();
	}

	void Renderer::AssignShadowSlots(const matrix4& cameraViewProj, const Loopie::CameraProjection& camProj, const AABB& sceneAABB)
	{
		s_ShadowCount = 0;
		for (int i = 0; i < MAX_SHADOW_CASTING_LIGHTS; ++i)
		{
			s_ShadowSlots[i].lightIndex = -1;
			s_ShadowSlots[i].rawLightIndex = -1;
		}

		int activeLightCount = 0;
		for (int j = 0; j < s_LightCount; ++j)
		{
			Light* l = s_Lights[j];
			if (!l->GetIsActive())
				continue;

			Loopie::LightType lType = l->GetLightType();

			
			bool canCastShadow = l->GetCastsShadows() && lType != LightType::Ambient
													  && lType != LightType::Point; // May be TODO, but it's very hard. I think beyond our scope.
			if (canCastShadow)
			{
				if (lType == LightType::Directional)
				{
					if (s_ShadowCount < MAX_SHADOW_CASTING_LIGHTS)
					{
						s_ShadowSlots[s_ShadowCount].lightIndex = activeLightCount;
						s_ShadowSlots[s_ShadowCount].rawLightIndex = j;
						s_ShadowSlots[s_ShadowCount].dynamicLightSpaceMatrix = ComputeDirectionalLightMatrix(cameraViewProj, l->GetTransform()->Forward(), camProj);
						matrix4 newStaticMatrix = ComputeDirectionalLightMatrixFromAABB(sceneAABB, l->GetTransform()->Forward());
						if (newStaticMatrix != s_ShadowSlots[s_ShadowCount].staticLightSpaceMatrix)
						{
							s_ShadowSlots[s_ShadowCount].staticLightSpaceMatrix = newStaticMatrix;
							s_ShadowSlots[s_ShadowCount].isDirty = true;
						}
						s_ShadowCount++;
					}
					else
					{
						Log::Warn("Unable to cast shadows on all requested lights...");
					}
				}
				else if (lType == LightType::Spot)
				{
					if (s_ShadowCount < MAX_SHADOW_CASTING_LIGHTS)
					{
						s_ShadowSlots[s_ShadowCount].lightIndex = activeLightCount;
						s_ShadowSlots[s_ShadowCount].rawLightIndex = j;
						s_ShadowSlots[s_ShadowCount].dynamicLightSpaceMatrix = l->GetLightSpaceMatrix();
						matrix4 newStaticMatrix = l->GetLightSpaceMatrix();
						if (newStaticMatrix != s_ShadowSlots[s_ShadowCount].staticLightSpaceMatrix)
						{
							s_ShadowSlots[s_ShadowCount].staticLightSpaceMatrix = newStaticMatrix;
							s_ShadowSlots[s_ShadowCount].isDirty = true;
						}
						s_ShadowCount++;
					}
					else
					{
						Log::Warn("Unable to cast shadows on all requested lights...");
					}
				}

			}

			activeLightCount++;
		}
	}

	void Renderer::SetShadowsDirty()
	{
		for (int i = 0; i < MAX_SHADOW_CASTING_LIGHTS; ++i)
		{
			s_ShadowSlots[i].isDirty = true;
		}
	}

	bool Renderer::BeginDynamicShadowPass(int shadowSlotIndex)
	{
		ShadowSlot& sl = s_ShadowSlots[shadowSlotIndex];
		if (sl.lightIndex < 0)
		{
			return false;
		}

		EnableDepth();
		EnableDepthMask();
		
		switch (s_Lights[sl.rawLightIndex]->GetLightType())
		{
		case LightType::Directional:
		case LightType::Spot:
			sl.dynamicMap->Bind();
			SetViewport(0, 0, sl.dynamicMap->GetWidth(), sl.dynamicMap->GetHeight());
			sl.dynamicMap->Clear();
			s_ShadowMapShader->Bind();
			s_ShadowMapShader->SetUniformInt("lp_ShadowSlotIndex", shadowSlotIndex);
			s_ShadowingUniformBuffer->SetData(&sl.dynamicLightSpaceMatrix, shadowSlotIndex);
			break;
		case LightType::Point:
			Log::Warn("Point lights' shadows not implemented yet.");
			break;
		case LightType::Ambient:
		default:
			Log::Warn("Tried to cast shadows of ambient or unknown type light.");
			return false;
		}
		return true;
	}

	bool Renderer::BeginStaticShadowPass(int shadowSlotIndex)
	{
		ShadowSlot& sl = s_ShadowSlots[shadowSlotIndex];
		if (sl.lightIndex < 0)
		{
			return false;
		}

		if (!sl.isDirty)
		{
			return false;
		}

		EnableDepth();
		EnableDepthMask();

		switch (s_Lights[sl.rawLightIndex]->GetLightType())
		{
		case LightType::Directional:
		case LightType::Spot:
			sl.staticMap->Bind();
			SetViewport(0, 0, sl.staticMap->GetWidth(), sl.staticMap->GetHeight());
			sl.staticMap->Clear();
			s_ShadowMapShader->Bind();
			s_ShadowMapShader->SetUniformInt("lp_ShadowSlotIndex", shadowSlotIndex);
			s_ShadowingUniformBuffer->SetData(&sl.staticLightSpaceMatrix, shadowSlotIndex);
			break;
		case LightType::Point:
			Log::Warn("Point lights' shadows not implemented yet.");
			break;
		case LightType::Ambient:
		default:
			Log::Warn("Tried to cast shadows of ambient or unknown type light.");
			return false;
		}
		return true;
	}

	void Renderer::FlushShadowItem(std::shared_ptr<VertexArray> vao, const Transform* transform, const std::vector<matrix4>& bones)
	{
		vao->Bind();
		s_ShadowMapShader->SetUniformMat4("lp_Transform", transform->GetLocalToWorldMatrix());
		s_ShadowMapShader->SetUniformInt("lp_Skinned", !bones.empty() ? 1 : 0);
		if (!bones.empty())
		{
			int boneOffset = (int)UploadBones(bones);
			s_ShadowMapShader->SetUniformInt("lp_BoneOffset", boneOffset);
		}
		glDrawElements(GL_TRIANGLES, vao->GetIndexBuffer().GetCount(), GL_UNSIGNED_INT, nullptr);
		vao->Unbind();
	}

	void Renderer::EndDynamicShadowPass(int shadowSlotIndex)
	{
		s_ShadowSlots[shadowSlotIndex].dynamicMap->Unbind();
		s_ShadowMapShader->Unbind();
	}

	void Renderer::EndStaticShadowPass(int shadowSlotIndex)
	{
		s_ShadowSlots[shadowSlotIndex].staticMap->Unbind();
		s_ShadowMapShader->Unbind();
		s_ShadowSlots[shadowSlotIndex].isDirty = false;
	}

	void Renderer::BindStaticShadowTexturesForMainPass()
	{
		for (int i = 0; i < MAX_SHADOW_CASTING_LIGHTS; ++i)
		{
			s_ShadowSlots[i].staticMap->BindTexture(STATIC_SHADOW_TEXTURE_INDEX + i);
		}
	}

	void Renderer::BindDynamicShadowTexturesForMainPass()
	{
		for (int i = 0; i < MAX_SHADOW_CASTING_LIGHTS; ++i)
		{
			s_ShadowSlots[i].dynamicMap->BindTexture(DYNAMIC_SHADOW_TEXTURE_INDEX +i);
		}
	}

	int Renderer::GetShadowCastingLightCount()
	{
		return int(s_ShadowCount);
	}

	matrix4 Renderer::GetShadowSlotMatrix(unsigned int slotIndex)
	{
		if (slotIndex >= MAX_SHADOW_CASTING_LIGHTS || s_ShadowSlots[slotIndex].lightIndex < 0)
			return matrix4(1.0f);
		return s_ShadowSlots[slotIndex].dynamicLightSpaceMatrix;
	}

	matrix4 Renderer::ComputeDirectionalLightMatrix(const matrix4& cameraViewProj, const vec3& lightDir, const Loopie::CameraProjection& camProj)
	{
		vec3 v[8];
		for (int n = 0; n < 8; ++n) 
		{
			v[n].x = (n & 1) ? 1.0f : -1.0f;
			v[n].y = (n & 2) ? 1.0f : -1.0f;
			v[n].z = (n & 4) ? 1.0f : -1.0f;
		}

		vec4 worldV[8];
		for (int i = 0; i < 8; ++i)
		{
			worldV[i] = inverse(cameraViewProj) * vec4(v[i], 1.0f);
			worldV[i] /= worldV[i].w;
		}

		float shadowRatio = (camProj == CameraProjection::Orthographic) ? 1.0f : 0.10f;
		for (int i = 0; i < 8; ++i)
		{
			// Corners with z = +1 are the far corners (indices 4-7, where bit 4 is set)
			if (v[i].z > 0.0f)
			{
				vec3 nearCorner = vec3(worldV[i - 4]);  // corresponding near corner
				vec3 farCorner = vec3(worldV[i]);
				worldV[i] = vec4(glm::mix(nearCorner, farCorner, shadowRatio), 1.0f);
			}
		}

		vec3 centroid = vec3(0.0f);
		for (int j = 0; j < 8; ++j)
		{
			centroid += vec3(worldV[j]);
		}
		centroid /= 8.0f;

		vec3 eye = centroid - lightDir * 100.0f;
		vec3 upVec = abs(dot(lightDir, vec3(0.0f, 1.0f, 0.0f))) > 0.99 ? vec3(1.0f, 0.0f, 0.0f) : vec3(0.0f, 1.0f, 0.0f);
		matrix4 lightView = glm::lookAtLH(eye, centroid, upVec);

		
		vec3 minBounds = vec3(lightView * worldV[0]);
		vec3 maxBounds = minBounds;

		for (int k = 1; k < 8; ++k)
		{
			vec3 p = vec3(lightView * worldV[k]);
			minBounds = glm::min(minBounds, p);
			maxBounds = glm::max(maxBounds, p);
		}

		matrix4 orthogonalMat = glm::orthoLH(minBounds.x, maxBounds.x, minBounds.y, maxBounds.y, minBounds.z - 50.0f, maxBounds.z);
		return orthogonalMat * lightView;
	}

	matrix4 Renderer::ComputeDirectionalLightMatrixFromAABB(const AABB& sceneBounds, const vec3& lightDir)
	{
		vec3 v[8];
		for (int n = 0; n < 8; ++n)
		{
			v[n].x = (n & 1) ? sceneBounds.MaxPoint.x : sceneBounds.MinPoint.x;
			v[n].y = (n & 2) ? sceneBounds.MaxPoint.y : sceneBounds.MinPoint.y;
			v[n].z = (n & 4) ? sceneBounds.MaxPoint.z : sceneBounds.MinPoint.z;
		}		

		vec3 centroid = vec3(0.0f);
		for (int j = 0; j < 8; ++j)
		{
			centroid += v[j];
		}
		centroid /= 8.0f;

		vec3 eye = centroid - lightDir * 100.0f;
		vec3 upVec = abs(dot(lightDir, vec3(0.0f, 1.0f, 0.0f))) > 0.99 ? vec3(1.0f, 0.0f, 0.0f) : vec3(0.0f, 1.0f, 0.0f);
		matrix4 lightView = glm::lookAtLH(eye, centroid, upVec);

		vec3 minBounds = vec3(lightView * vec4(v[0], 1.0f));
		vec3 maxBounds = minBounds;

		for (int k = 1; k < 8; ++k)
		{
			vec3 p = vec3(lightView * vec4(v[k], 1.0f));
			minBounds = glm::min(minBounds, p);
			maxBounds = glm::max(maxBounds, p);
		}

		matrix4 orthogonalMat = glm::orthoLH(minBounds.x, maxBounds.x, minBounds.y, maxBounds.y, minBounds.z - 50.0f, maxBounds.z);
		return orthogonalMat * lightView;
	}

	void Renderer::SetShadowQuality(ShadowQuality q)
	{
		s_PendingShadowQuality = q;
	}

	void Renderer::SetShadowFilter(ShadowFilter f)
	{
		s_PendingShadowFilter = f;
	}

	void Renderer::ApplyPendingShadowSettings()
	{
		if (s_ShadowSettings.GetShadowQuality() != s_PendingShadowQuality)
		{
			//	mark every slot dirty(so statics re - bake)
			s_ShadowSettings.SetShadowQuality(s_PendingShadowQuality);
			BuildShadowMaps();
			SetShadowsDirty();
		}

		if (s_ShadowSettings.GetFilter() != s_PendingShadowFilter)
		{
			s_ShadowSettings.SetFilter(s_PendingShadowFilter);
		}
	}

	void Renderer::RegisterCamera(Camera& camera) {
		auto it = std::find(s_RenderCameras.begin(), s_RenderCameras.end(), &camera);
		if (it == s_RenderCameras.end()) {
			s_RenderCameras.push_back(&camera);
		}
	}

	void Renderer::UnregisterCamera(Camera& camera) {

		auto it = std::find(s_RenderCameras.begin(), s_RenderCameras.end(), &camera);
		if (it != s_RenderCameras.end()) {
			s_RenderCameras.erase(it);
		}
	}

	void Renderer::BeginScene(const matrix4& viewMatrix, const matrix4& projectionMatrix, bool gizmo)
	{
		ApplyPendingShadowSettings();
		s_UseGizmos = gizmo;
		s_MatricesUniformBuffer->SetData(&projectionMatrix[0][0], 0);
		s_MatricesUniformBuffer->SetData(&viewMatrix[0][0], 1);
		vec3 cameraPos = vec3(glm::inverse(viewMatrix)[3]);

		int activeLightCount = 0;
		for (int i = 0; i < s_LightCount; ++i)
		{
			const Light& l = *s_Lights[i];
			if (!l.GetIsActive())
			{
				continue;
			}
			vec4 colorIntensity = vec4(l.GetColor(), l.GetIntensity());
			vec4 positionType = vec4(l.GetTransform()->GetWorldPosition(), l.GetLightType());
			vec4 directionInnerCone = vec4(l.GetTransform()->Forward(), l.GetInnerConeAngle());
			vec4 attenuationOuterCone = vec4(l.GetAttenuationConstant(), l.GetAttenuationLinear(), l.GetAttenuationQuadratic(), l.GetOuterConeAngle());
			
			int shadowMapIndex = -1;
			if (l.GetCastsShadows())
			{
				for (int j = 0; j < MAX_SHADOW_CASTING_LIGHTS; ++j)
				{
					if (s_ShadowSlots[j].lightIndex == activeLightCount)
					{
						shadowMapIndex = j;
						break;
					}
				}
			}
			vec4 sMapAndSColor = vec4(shadowMapIndex, l.GetShadowColor());
			
			s_LightingUniformBuffer->SetData(&colorIntensity,		activeLightCount * 5 + 1);
			s_LightingUniformBuffer->SetData(&positionType,			activeLightCount * 5 + 2);
			s_LightingUniformBuffer->SetData(&directionInnerCone,	activeLightCount * 5 + 3);
			s_LightingUniformBuffer->SetData(&attenuationOuterCone, activeLightCount * 5 + 4);
			s_LightingUniformBuffer->SetData(&sMapAndSColor,		activeLightCount * 5 + 5);

			activeLightCount++;
		}

		vec4 cameraPosLightCount = vec4(cameraPos, activeLightCount);
		s_LightingUniformBuffer->SetData(&cameraPosLightCount, 0);

		BindDynamicShadowTexturesForMainPass();
		BindStaticShadowTexturesForMainPass();

		for (int i = 0; i < MAX_SHADOW_CASTING_LIGHTS; ++i)
		{
			s_StaticMatricesUniformBuffer->SetData(&s_ShadowSlots[i].staticLightSpaceMatrix, i);
		}

		s_BoneBufferOffset = 0;

		s_FrameUpdatedShaders.clear();

		if (s_UseGizmos)
			Gizmo::BeginGizmo();
	}

	void Renderer::EndScene()
	{
		FlushRenderQueue();

		Gizmo::EndGizmo();
	}

	void Renderer::AddRenderItem(std::shared_ptr<VertexArray> vao, std::shared_ptr<Material> material, const Transform* transform, const std::vector<matrix4>& bones)
	{
		if(material->GetHasTransparency())
			s_TransparentRenderQueue.emplace_back(RenderItem{ vao, vao->GetIndexBuffer().GetCount(), material, transform, bones});
		else
			s_OpaqueRenderQueue.emplace_back(RenderItem{ vao, vao->GetIndexBuffer().GetCount(), material, transform, bones});
	}

	void Renderer::FlushRenderItem(std::shared_ptr<VertexArray> vao, std::shared_ptr<Material> material, const Transform* transform, const std::vector<matrix4>& bones)
	{
		FlushRenderItem(vao, material, transform->GetLocalToWorldMatrix(), bones);
	}

	void Renderer::FlushRenderItem(std::shared_ptr<VertexArray> vao, std::shared_ptr<Material> material, const matrix4& modelMatrix, const std::vector<matrix4>& bones)
	{
		vao->Bind();
		material->Bind();
		SetFrameUniforms(material->GetShader());
		SetRenderUniforms(material, modelMatrix, bones);
		glDrawElements(GL_TRIANGLES, vao->GetIndexBuffer().GetCount(), GL_UNSIGNED_INT, nullptr);
		vao->Unbind();
	}

	void Renderer::FlushOpaqueRenderQueue()
	{
		std::sort(s_OpaqueRenderQueue.begin(), s_OpaqueRenderQueue.end(), CompareByMaterial);

		for (const RenderItem& item : s_OpaqueRenderQueue) {
			item.VAO->Bind();
			item.Material->Bind();
			SetFrameUniforms(item.Material->GetShader());
			SetRenderUniforms(item.Material, item.Transform, item.Bones);
			glDrawElements(GL_TRIANGLES, item.IndexCount, GL_UNSIGNED_INT, nullptr);
			item.VAO->Unbind();
		}

		s_OpaqueRenderQueue.clear();
	}

	void Renderer::FlushTransparentRenderQueue()
	{
		glTextureBarrier();
		glActiveTexture(GL_TEXTURE0 + 12); // 12 = Scene Depth Texture Slot, hard-coded
		glBindTexture(GL_TEXTURE_2D, s_SceneDepthTextureID);

		EnableBlend();
		DisableDepthMask();
		for (const RenderItem& item : s_TransparentRenderQueue) {
			item.VAO->Bind();
			item.Material->Bind();
			SetFrameUniforms(item.Material->GetShader());
			SetRenderUniforms(item.Material, item.Transform, item.Bones);
			glDrawElements(GL_TRIANGLES, item.IndexCount, GL_UNSIGNED_INT, nullptr);
			item.VAO->Unbind();
		}
		EnableDepthMask();
		DisableBlend();

		s_TransparentRenderQueue.clear();
	}

	void Renderer::FlushRenderQueue()
	{
		/// SORT By Material

		// *** Sorting in FlushRendererQueue *** - PSS 09/04/2026 - Since we're not yet
		// rendering by material, I decided to do it by transparency.
		// There's a check in the Material -> Material Has Transparency.
		// Items without transparency are drawn before items with transparency.

		FlushOpaqueRenderQueue();
		FlushTransparentRenderQueue();
	}

	void Renderer::ClearParticles()
	{
		s_ParticleQueue.clear();
		s_ParticlesData.transformsBatch.clear();
		s_ParticlesData.colorsBatch.clear();
	}

	void Renderer::AddParticle(const matrix4& transform, const vec4& color, float depth, std::shared_ptr<Material> material, std::shared_ptr<Texture> sprite, std::shared_ptr<VertexArray> vao)
	{
		s_ParticleQueue.push_back({ transform, color, depth, material, sprite, vao });
	}

	void Renderer::FlushParticles()
	{
		if (s_ParticleQueue.empty()) return;

		std::sort(s_ParticleQueue.begin(), s_ParticleQueue.end(), [](const ParticleSortData& a, const ParticleSortData& b) {
			return a.depth > b.depth;
			});

		int batchStart = 0;

		UniformValue useSpriteVal;
		useSpriteVal.type = UniformType_bool;

		for (size_t i = 0; i <= s_ParticleQueue.size(); i++)
		{
			bool isEnd = (i == s_ParticleQueue.size());
			bool stateChanged = false;

			if (!isEnd && i > batchStart) {
				const auto& prev = s_ParticleQueue[i - 1];
				const auto& curr = s_ParticleQueue[i];
				if (prev.sprite != curr.sprite) {
					stateChanged = true;
				}
			}

			if (stateChanged || isEnd)
			{
				int batchSize = i - batchStart;
				if (batchSize == 0) 
					continue;

				auto& firstParticle = s_ParticleQueue[batchStart];

				s_ParticlesData.transformsBatch.clear();
				s_ParticlesData.colorsBatch.clear();
				for (size_t j = batchStart; j < i; j++) {
					s_ParticlesData.transformsBatch.push_back(s_ParticleQueue[j].transform);
					s_ParticlesData.colorsBatch.push_back(s_ParticleQueue[j].color);
				}

				firstParticle.vao->Bind();
				s_ParticlesData.TransformVBO->SetData(s_ParticlesData.transformsBatch.data(), s_ParticlesData.transformsBatch.size() * sizeof(matrix4));

				for (int k = 0; k < 4; k++) {
					glEnableVertexAttribArray(2 + k);
					glVertexAttribPointer(2 + k, 4, GL_FLOAT, GL_FALSE, sizeof(matrix4), (void*)(sizeof(vec4) * k));
					glVertexAttribDivisor(2 + k, 1);
				}

				s_ParticlesData.ColorVBO->SetData(s_ParticlesData.colorsBatch.data(), s_ParticlesData.colorsBatch.size() * sizeof(vec4));
				glEnableVertexAttribArray(6);
				glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(vec4), (void*)0);
				glVertexAttribDivisor(6, 1);

				useSpriteVal.value = (firstParticle.sprite != nullptr);

				if (firstParticle.sprite) {
					firstParticle.material->SetTexture("u_Sprite", firstParticle.sprite);
				}
				firstParticle.material->SetShaderVariable("u_UseSprite", useSpriteVal);

				firstParticle.material->Bind();
				SetFrameUniforms(firstParticle.material->GetShader());

				glDrawElementsInstanced(GL_TRIANGLES, firstParticle.vao->GetIndexBuffer().GetCount(), GL_UNSIGNED_INT, nullptr, batchSize);

				firstParticle.vao->Unbind();

				batchStart = i;
			}
		}

		s_ParticleQueue.clear();
	}

	unsigned int Renderer::UploadBones(const std::vector<matrix4>& bones)
	{
		if (bones.empty())
			return 0;

		unsigned int size = bones.size() * sizeof(matrix4);

		// safety check
		if (s_BoneBufferOffset + size > s_BoneBufferCapacity)
		{
			Log::Error("Bone SSBO overflow! Increase buffer size.");
			return 0;
		}

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, s_BonesSSBO->GetRendererID());
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, s_BoneBufferOffset, size, bones.data());

		unsigned int offset = s_BoneBufferOffset / sizeof(matrix4);
		s_BoneBufferOffset += size;

		return offset;
	}

	void Renderer::SetFrameUniforms(Shader& shader)
	{
		if (s_FrameUpdatedShaders.find(&shader) != s_FrameUpdatedShaders.end())
			return;

		// ---- GLOBAL UNIFORMS ----
		if (shader.GetUniformLocation("lp_Time") != -1)
			shader.SetUniformFloat("lp_Time", Time::GetRunTime());

		if (shader.GetUniformLocation("lp_SceneDepth") != -1)
			shader.SetUniformInt("lp_SceneDepth", 12);

		if (shader.GetUniformLocation("lp_Near") != -1)
			shader.SetUniformFloat("lp_Near", s_NearPlane);

		if (shader.GetUniformLocation("lp_Far") != -1)
			shader.SetUniformFloat("lp_Far", s_FarPlane);

		if (shader.GetUniformLocation("lp_ShadowKernelRadius") != -1)
			shader.SetUniformInt("lp_ShadowKernelRadius", s_ShadowSettings.GetFilterRadius());

		for (int i = 0; i < MAX_SHADOW_CASTING_LIGHTS; ++i)
		{
			std::string name = "lp_ShadowMaps[" + std::to_string(i) + "]";
			if (shader.GetUniformLocation(name) != -1)
				shader.SetUniformInt(name, DYNAMIC_SHADOW_TEXTURE_INDEX + i);
		}

		for (int i = 0; i < MAX_SHADOW_CASTING_LIGHTS; ++i)
		{
			std::string name = "lp_StaticShadowMaps[" + std::to_string(i) + "]";
			if (shader.GetUniformLocation(name) != -1)
				shader.SetUniformInt(name, STATIC_SHADOW_TEXTURE_INDEX + i);
		}

		s_FrameUpdatedShaders.insert(&shader);
	}

	void Renderer::SetRenderUniforms(std::shared_ptr<Material> material, const Transform* transform, const std::vector<matrix4>& bones)
	{
		SetRenderUniforms(material, transform->GetLocalToWorldMatrix(), bones);
	}

	void Renderer::BuildShadowMaps()
	{
		for (int i = 0; i < MAX_SHADOW_CASTING_LIGHTS; ++i)
		{
			s_ShadowSlots[i].dynamicMap = std::make_shared<ShadowMap>(s_ShadowSettings.GetDynamicRes(), s_ShadowSettings.GetDynamicRes());
			s_ShadowSlots[i].staticMap = std::make_shared<ShadowMap>(s_ShadowSettings.GetStaticRes(), s_ShadowSettings.GetStaticRes());
		}
	}

	void Renderer::SetRenderUniforms(std::shared_ptr<Material> material, const matrix4& modelMatrix, const std::vector<matrix4>& bones)
	{
		Shader& shader = material->GetShader();

		if (shader.GetUniformLocation("lp_Transform") != -1)
			shader.SetUniformMat4("lp_Transform", modelMatrix);

		if (shader.GetUniformLocation("lp_Skinned") != -1)
			shader.SetUniformInt("lp_Skinned", !bones.empty() ? 1 : 0);

		if (!bones.empty())
		{
			int boneOffset = (int)UploadBones(bones);
			shader.SetUniformInt("lp_BoneOffset", boneOffset);
		}
	}

	void Renderer::BlendFunction(BlendFactorMode src, BlendFactorMode dst)
	{
		glBlendFunc((unsigned int)src, (unsigned int)dst);
	}

	void Renderer::BlendEquation(BlendEquationMode eq)
	{
		glBlendEquation((GLenum)eq);
	}

	void Renderer::EnableDepth()
	{
		glEnable(GL_DEPTH_TEST);
	}

	void Renderer::DisableDepth()
	{
		glDisable(GL_DEPTH_TEST);
	}

	void Renderer::SetDepthFunc(DepthFunc cond)
	{
		glDepthFunc((unsigned int)cond);
	}

	void Renderer::EnableDepthMask()
	{
		glDepthMask(GL_TRUE);
	}

	void Renderer::DisableDepthMask()
	{
		glDepthMask(GL_FALSE);
	}

	void Renderer::EnableStencil()
	{
		glEnable(GL_STENCIL_TEST);
	}

	void Renderer::DisableStencil()
	{
		glDisable(GL_STENCIL_TEST);
	}

	void Renderer::SetStencilMask(unsigned int mask)
	{
		glStencilMask(mask);
	}

	void Renderer::SetStencilOp(StencilOp stencil_fail, StencilOp depth_fail, StencilOp pass)
	{
		glStencilOp((unsigned int)stencil_fail, (unsigned int)depth_fail, (unsigned int)pass);
	}

	void Renderer::SetStencilFunc(StencilFunc cond, int ref, unsigned int mask)
	{
		glStencilFunc((unsigned int)cond, ref, mask);
	}

	void Renderer::EnableCulling()
	{
		glEnable(GL_CULL_FACE);
	}

	void Renderer::DisableCulling()
	{
		glDisable(GL_CULL_FACE);
	}

	void Renderer::CullFace(CullFaceMode mode)
	{
		glCullFace((unsigned int)mode);
	}

	void Renderer::EnableBlend()
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	void Renderer::DisableBlend()
	{
		glDisable(GL_BLEND);
	}

	void Renderer::SetDepthWrite(bool enable)
	{
		glDepthMask(enable ? GL_TRUE : GL_FALSE);
	}

	void Renderer::SaveRenderSettintgs()
	{
		JsonData configData = ProjectConfig::GetData();
		JsonNode engineConfig;
		if (configData.HasKey("", "engine_config"))
		{
			engineConfig = configData.Child("engine_config");
		}
		else
		{
			engineConfig = configData.CreateObjectField("engine_config");
		}

		engineConfig.CreateField<int>("shadow_quality", static_cast<int>(s_ShadowSettings.GetShadowQuality()));
		engineConfig.CreateField<int>("shadow_filter", static_cast<int>(s_ShadowSettings.GetFilter()));
		engineConfig.CreateField<float>("exposure", static_cast<float>(s_Exposure));
		engineConfig.CreateField<float>("bloom_threshold", static_cast<float>(s_BloomThreshold));
		engineConfig.CreateField<float>("bloom_strength", static_cast<float>(s_BloomStrength));

		ProjectConfig::Save(configData);
	}

	void Renderer::InitPostProcessing()
	{
		s_TonemapShader = std::make_unique<Shader>("assets/shaders/Tonemap.shader");
		if (!s_TonemapShader->GetIsValidShader())
		{
			Log::Error("Tonemap shader failed to compile!");
		}
		s_BloomExtractShader = std::make_unique<Shader>("assets/shaders/BloomExtract.shader");
		if (!s_BloomExtractShader->GetIsValidShader())
		{
			Log::Error("Bloom Extract shader failed to compile!");
		}
		s_BloomDownsampleShader = std::make_unique<Shader>("assets/shaders/BloomDownsample.shader");
		if (!s_BloomDownsampleShader->GetIsValidShader())
		{
			Log::Error("Bloom Downsample shader failed to compile!");
		}
		s_BloomUpsampleShader = std::make_unique<Shader>("assets/shaders/BloomUpsample.shader");
		if (!s_BloomUpsampleShader->GetIsValidShader())
		{
			Log::Error("Bloom Upsample shader failed to compile!");
		}
		glGenVertexArrays(1, &s_DummyVAO);
	}

	// Passes the hdr texture into an ldr one
	void Renderer::TonemapPass(unsigned int hdrTextureID, FrameBuffer& ldrTarget, unsigned int bloomTextureID)
	{
		ldrTarget.Bind();
		SetViewport(0, 0, ldrTarget.GetWidth(), ldrTarget.GetHeight());
		DisableDepth();
		s_TonemapShader->Bind();
		
		glBindVertexArray(s_DummyVAO);
		glActiveTexture(GL_TEXTURE0 + TONEMAP_TEXTURE_INDEX);
		glBindTexture(GL_TEXTURE_2D, hdrTextureID);
		s_TonemapShader->SetUniformInt("lp_HDRBuffer", TONEMAP_TEXTURE_INDEX);
		
		glActiveTexture(GL_TEXTURE0 + BLOOM_TEXTURE_INDEX);
		glBindTexture(GL_TEXTURE_2D, bloomTextureID);
		s_TonemapShader->SetUniformInt("lp_BloomBuffer", BLOOM_TEXTURE_INDEX);

		s_TonemapShader->SetUniformFloat("lp_Exposure", s_Exposure);
		s_TonemapShader->SetUniformFloat("lp_BloomStrength", s_BloomStrength); 
		glDrawArrays(GL_TRIANGLES, 0, 3);

		EnableDepth();
		s_TonemapShader->Unbind();
		ldrTarget.Unbind();
		glActiveTexture(GL_TEXTURE0);
	}

	float Renderer::GetExposure()
	{
		return s_Exposure;
	}

	void Renderer::SetExposure(float exposure)
	{
		if (exposure < MIN_EXPOSURE_VALUE)
		{
			s_Exposure = MIN_EXPOSURE_VALUE;
		}
		else if (exposure > MAX_EXPOSURE_VALUE)
		{
			s_Exposure = MAX_EXPOSURE_VALUE;
		}
		else
		{
			s_Exposure = exposure;
		}
	}

	void Renderer::SetBloomThreshold(float bloomThreshold)
	{
		s_BloomThreshold = bloomThreshold;
	}

	float Renderer::GetBloomThreshold()
	{
		return s_BloomThreshold;
	}

	void Renderer::SetBloomStrength(float bloomStrength)
	{
		s_BloomStrength = bloomStrength;
	}

	float Renderer::GetBloomStrength()
	{
		return s_BloomStrength;
	}

	void Renderer::BloomExtractPass(unsigned int hdrTextureID)
	{
		EnsureBloomChain(s_CurrentViewport.z, s_CurrentViewport.w);
		FrameBuffer& dst = *s_BloomChain[0];  
		dst.Bind();
		SetViewport(0, 0, dst.GetWidth(), dst.GetHeight());
		DisableDepth();
		s_BloomExtractShader->Bind();

		glBindVertexArray(s_DummyVAO);
		glActiveTexture(GL_TEXTURE0 + BLOOM_TEXTURE_INDEX);   
		glBindTexture(GL_TEXTURE_2D, hdrTextureID);
		s_BloomExtractShader->SetUniformInt("lp_hdrBuffer", BLOOM_TEXTURE_INDEX);
		s_BloomExtractShader->SetUniformFloat("lp_BloomThreshold", s_BloomThreshold);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		EnableDepth();
		s_BloomExtractShader->Unbind();
		dst.Unbind();
		glActiveTexture(GL_TEXTURE0);
	}

	void Renderer::BloomDownsamplePass()
	{
		DisableDepth();
		s_BloomDownsampleShader->Bind();
		glBindVertexArray(s_DummyVAO);

		for (size_t i = 1; i < s_BloomChain.size(); ++i)
		{
			FrameBuffer& dst = *s_BloomChain[i];
			FrameBuffer& src = *s_BloomChain[i - 1];

			dst.Bind();
			SetViewport(0, 0, dst.GetWidth(), dst.GetHeight());
			glActiveTexture(GL_TEXTURE0 + BLOOM_TEXTURE_INDEX);
			glBindTexture(GL_TEXTURE_2D, src.GetTextureId());
			s_BloomDownsampleShader->SetUniformInt("lp_SourceTexture", BLOOM_TEXTURE_INDEX);
			s_BloomDownsampleShader->SetUniformVec2("lp_SrcTexelSize", glm::vec2({ 1.0f / src.GetWidth(), 1.0f / src.GetHeight() }));
			glDrawArrays(GL_TRIANGLES, 0, 3);
			dst.Unbind();
		}

		EnableDepth();
		s_BloomDownsampleShader->Unbind();
		glActiveTexture(GL_TEXTURE0);
	}

	void Renderer::BloomUpsamplePass()
	{
		EnableBlend();
		BlendFunction(BlendFactorMode::ONE, BlendFactorMode::ONE);

		s_BloomUpsampleShader->Bind();
		glBindVertexArray(s_DummyVAO);

		for (int i = BLOOM_MIP_COUNT - 1; i > 0; --i)
		{
			FrameBuffer& src = *s_BloomChain[i];    
			FrameBuffer& dst = *s_BloomChain[i - 1];

			dst.Bind();
			SetViewport(0, 0, dst.GetWidth(), dst.GetHeight());

			glActiveTexture(GL_TEXTURE0 + BLOOM_TEXTURE_INDEX);
			glBindTexture(GL_TEXTURE_2D, src.GetTextureId());
			s_BloomUpsampleShader->SetUniformInt("lp_SourceTexture", BLOOM_TEXTURE_INDEX);

			glDrawArrays(GL_TRIANGLES, 0, 3);
		}

		DisableBlend();
		s_BloomUpsampleShader->Unbind();
		glActiveTexture(GL_TEXTURE0);
	}

	void Renderer::EnsureBloomChain(unsigned int fullWidth, unsigned int fullHeight)
	{
		unsigned int newWidth = fullWidth >> 1;
		unsigned int newHeight = fullHeight >> 1;
		if (s_BloomChain.size() == BLOOM_MIP_COUNT)
		{
			if (s_BloomChain[0]->GetWidth() == newWidth && s_BloomChain[0]->GetHeight() == newHeight)
			{
				return;
			}
		}

		s_BloomChain.clear();
		for (int i = 0; i < BLOOM_MIP_COUNT; ++i)
		{
			unsigned int w = glm::max((unsigned int)1, fullWidth >> (i + 1));	// using max to ensure the texture doesn't become size 0
			unsigned int h = glm::max((unsigned int)1, fullHeight >> (i + 1));
			s_BloomChain.push_back(std::make_shared<FrameBuffer>(w, h, FrameBuffer::FrameBufferFormat::RGBA16F, false));
		}
	}

	std::vector<std::shared_ptr<FrameBuffer>> Renderer::GetBloomChain()
	{
		return s_BloomChain;
	}
}
