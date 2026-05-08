#pragma once
#include "Loopie/Math/MathTypes.h"
#include "Loopie/Resources/Types/Material.h"
#include "Loopie/Resources/Types/Texture.h"
#include "Loopie/Render/VertexArray.h"
#include "Loopie/Render/UniformBuffer.h"
#include "Loopie/Render/ShaderStorageBuffer.h"
#include "Loopie/Components/Camera.h"
#include "Loopie/Components/Light.h"

#include <filesystem>
#include <unordered_set>

#define MAX_LIGHTS 16 // Can be increased if necessary. Watch out that performance though!
#define MAX_SHADOW_CASTING_LIGHTS 4 // Can be increased if necessary. Watch out that performance though!
#define MAX_BONES_TOTAL 30000 

#define MAX_PARTICLES 100000 

//#define STATIC_SHADOW_TEXTURE_DEFINITION 8192
#define STATIC_SHADOW_TEXTURE_DEFINITION 4096

#define DYNAMIC_SHADOW_TEXTURE_INDEX 8 
#define STATIC_SHADOW_TEXTURE_INDEX 13

namespace Loopie {
	class Transform;
	class ShadowMap;

	class Renderer {
	public:

		enum class StencilOp {
			KEEP = 0x1E00,        // GL_KEEP
			ZERO = 0x0000,        // GL_ZERO
			REPLACE = 0x1E01,     // GL_REPLACE
			INCR = 0x1E02,        // GL_INCR
			INCR_WRAP = 0x8507,   // GL_INCR_WRAP
			DECR = 0x1E03,        // GL_DECR
			DECR_WRAP = 0x8508,   // GL_DECR_WRAP
			INVERT = 0x150A       // GL_INVERT
		};

		enum class StencilFunc {
			NEVER = 0x0200,       // GL_NEVER
			LESS = 0x0201,        // GL_LESS
			LEQUAL = 0x0203,      // GL_LEQUAL
			GREATER = 0x0204,     // GL_GREATER
			GEQUAL = 0x0206,      // GL_GEQUAL
			EQUAL = 0x0202,       // GL_EQUAL
			NOTEQUAL = 0x0205,    // GL_NOTEQUAL
			ALWAYS = 0x0207       // GL_ALWAYS
		};

		enum class DepthFunc {
			NEVER = 0x0200,       // GL_NEVER
			LESS = 0x0201,        // GL_LESS
			EQUAL = 0x0202,       // GL_EQUAL
			LEQUAL = 0x0203,      // GL_LEQUAL
			GREATER = 0x0204,     // GL_GREATER
			NOTEQUAL = 0x0205,    // GL_NOTEQUAL
			GEQUAL = 0x0206,      // GL_GEQUAL
			ALWAYS = 0x0207       // GL_ALWAYS
		};

		enum class CullFaceMode {
			FRONT = 0x0404,        // GL_FRONT
			BACK = 0x0405,         // GL_BACK
			FRONT_AND_BACK = 0x0408 // GL_FRONT_AND_BACK
		};

		enum class BlendFactorMode {
			ZERO = 0x0000,                     // GL_ZERO
			ONE = 0x0001,                      // GL_ONE
			SRC_COLOR = 0x0300,                // GL_SRC_COLOR
			ONE_MINUS_SRC_COLOR = 0x0301,      // GL_ONE_MINUS_SRC_COLOR
			SRC_ALPHA = 0x0302,                // GL_SRC_ALPHA
			ONE_MINUS_SRC_ALPHA = 0x0303,      // GL_ONE_MINUS_SRC_ALPHA
			DST_ALPHA = 0x0304,                // GL_DST_ALPHA
			ONE_MINUS_DST_ALPHA = 0x0305,      // GL_ONE_MINUS_DST_ALPHA
			DST_COLOR = 0x0306,                // GL_DST_COLOR
			ONE_MINUS_DST_COLOR = 0x0307,      // GL_ONE_MINUS_DST_COLOR
			CONSTANT_COLOR = 0x8001,           // GL_CONSTANT_COLOR
			ONE_MINUS_CONSTANT_COLOR = 0x8002, // GL_ONE_MINUS_CONSTANT_COLOR
			CONSTANT_ALPHA = 0x8003,           // GL_CONSTANT_ALPHA
			ONE_MINUS_CONSTANT_ALPHA = 0x8004  // GL_ONE_MINUS_CONSTANT_ALPHA
		};

		enum class BlendEquationMode {
			ADD = 0x8006,              // GL_FUNC_ADD
			SUBTRACT = 0x800A,         // GL_FUNC_SUBTRACT
			REVERSE_SUBTRACT = 0x800B, // GL_FUNC_REVERSE_SUBTRACT
			MIN = 0x8007,              // GL_MIN
			MAX = 0x8008               // GL_MAX
		};


		struct RenderItem {
			std::shared_ptr<VertexArray> VAO;
			unsigned int IndexCount;

			std::shared_ptr<Material> Material;
			const Transform* Transform;
			std::vector<matrix4> Bones;
		};


		struct ParticlesData {
			std::shared_ptr<VertexBuffer> TransformVBO;
			std::shared_ptr<VertexBuffer> ColorVBO;

			std::vector<matrix4> transformsBatch;
			std::vector<vec4> colorsBatch;
		};

		struct ShadowSlot
		{
			std::shared_ptr<ShadowMap> dynamicMap = nullptr;
			std::shared_ptr<ShadowMap> staticMap = nullptr;
			matrix4 dynamicLightSpaceMatrix = matrix4(1.0f);
			matrix4 staticLightSpaceMatrix = matrix4(1.0f);
			short lightIndex = -1;        // dense active index, matches UBO position
			short rawLightIndex = -1;     // index into s_Lights[], for accessing the Light object
			bool isDirty = true;
		};

		struct ShaderBufferObject {
			std::shared_ptr<ShaderStorageBuffer> Buffer;
			bool Used;
		};

		static void Init(void* context);
		static void Shutdown();

		static void Clear();
		static void SetClearColor(const vec4& color);
		static void SetViewport(unsigned int x, unsigned int y, unsigned int width, unsigned int height);
		static const vec4& GetCurrentViewport() { return s_CurrentViewport; }

		static void RegisterLight(Light* light);
		static void UnregisterLight(Light* light);
		static void RemoveAllLights();
		static unsigned short GetLightCount() { return s_LightCount; }

		// static std::shared_ptr<ShadowMap> GetShadowMap() { return s_ShadowMap; }; // Could be implemented for debugging
		static void InitShadowMapping();
		static void AssignShadowSlots(const matrix4& cameraViewProj, const Loopie::CameraProjection& camProj, const AABB& sceneAABB);
		static void SetShadowsDirty();
		static bool BeginDynamicShadowPass(int shadowSlotIndex);
		static bool BeginStaticShadowPass(int shadowSlotIndex);
		static void FlushShadowItem(std::shared_ptr<VertexArray> vao, const Transform* transform, const std::vector<matrix4>& bones = {});
		static void EndDynamicShadowPass(int shadowSlotIndex);
		static void EndStaticShadowPass(int shadowSlotIndex);
		static void BindDynamicShadowTexturesForMainPass();
		static void BindStaticShadowTexturesForMainPass();
		static int GetShadowCastingLightCount();
		static matrix4 GetShadowSlotMatrix(unsigned int slotIndex);
		static matrix4 ComputeDirectionalLightMatrix(const matrix4& cameraViewProj, const vec3& lightDir, const Loopie::CameraProjection& camProj);
		static matrix4 ComputeDirectionalLightMatrixFromAABB(const AABB& sceneBounds, const vec3& lightDir);

		// For water foam usage
		static void SetSceneDepthTexture(unsigned int textureID) { s_SceneDepthTextureID = textureID; };
		static void SetSceneFrustrumValues(float nearPlane, float farPlane) { s_NearPlane = nearPlane; s_FarPlane = farPlane; };

		static void RegisterCamera(Camera& camera);
		static void UnregisterCamera(Camera& camera);
		static const std::vector<Camera*>& GetRendererCameras() { return s_RenderCameras; }
		static bool IsGizmoActive() { return s_UseGizmos; }

		static void BeginScene(const matrix4& viewMatrix, const matrix4& projectionMatrix, bool gizmo = true);
		static void EndScene();

		static void AddRenderItem(std::shared_ptr<VertexArray> vao, std::shared_ptr<Material> material, const Transform* transform, const std::vector<matrix4>& bones = {});
		static void FlushRenderItem(std::shared_ptr<VertexArray> vao, std::shared_ptr<Material> material, const Transform* transform, const std::vector<matrix4>& bones = {});
		static void FlushRenderItem(std::shared_ptr<VertexArray> vao, std::shared_ptr<Material> material, const matrix4& modelMatrix, const std::vector<matrix4>& bones = {});

		static void FlushOpaqueRenderQueue();
		static void FlushTransparentRenderQueue();

		static void FlushRenderQueue();
		
		static void ClearParticles();
		static void AddParticle(const matrix4& transform, const vec4& color);
		static void FlushParticles(std::shared_ptr<VertexArray> vao, std::shared_ptr<Material> material);

		static void EnableDepth();
		static void DisableDepth();
		static void SetDepthFunc(DepthFunc cond);
		static void EnableDepthMask();
		static void DisableDepthMask();

		static void EnableBlend();
		static void DisableBlend();
		static void BlendFunction(BlendFactorMode src, BlendFactorMode dst);
		static void BlendEquation(BlendEquationMode eq);

		static void EnableStencil();
		static void DisableStencil();
		static void SetStencilMask(unsigned int mask);
		static void SetStencilOp(StencilOp stencil_fail, StencilOp depth_fail, StencilOp pass);
		static void SetStencilFunc(StencilFunc cond, int ref, unsigned int mask);

		static void EnableCulling();
		static void DisableCulling();
		static void CullFace(CullFaceMode mode);

		static void SetDepthWrite(bool enable);

	private:
		static void SetFrameUniforms(Shader& shader);
		static void SetRenderUniforms(std::shared_ptr<Material> material, const Transform* transform, const std::vector<matrix4>& bones = {});
		static void SetRenderUniforms(std::shared_ptr<Material> material, const matrix4& modelMatrix, const std::vector<matrix4>& bones = {});
		

		static unsigned int UploadBones(const std::vector<matrix4>& bones);

	private:
		static std::vector<RenderItem> s_OpaqueRenderQueue;
		static std::vector<RenderItem> s_TransparentRenderQueue;
		static std::vector<Camera*> s_RenderCameras;
		static std::shared_ptr<UniformBuffer> s_MatricesUniformBuffer;
		static std::shared_ptr<UniformBuffer> s_LightingUniformBuffer;
		static std::shared_ptr<UniformBuffer> s_ShadowingUniformBuffer;
		static std::shared_ptr<UniformBuffer> s_StaticMatricesUniformBuffer;
		static std::unordered_set<Shader*> s_FrameUpdatedShaders;

		static std::shared_ptr<ShaderStorageBuffer> s_BonesSSBO;
		static unsigned int s_BoneBufferOffset;
		static unsigned int s_BoneBufferCapacity;

		static ParticlesData s_ParticlesData;

		static vec4 s_CurrentViewport;

		static bool s_UseGizmos;
		static Light* s_Lights[MAX_LIGHTS];
		static unsigned short s_LightCount;
		static ShadowSlot s_ShadowSlots[MAX_SHADOW_CASTING_LIGHTS];
		static unsigned short s_ShadowCount;
		static std::unique_ptr<Shader> s_ShadowMapShader;

		static unsigned int s_SceneDepthTextureID;
		static float s_NearPlane;
		static float s_FarPlane;

	};
}