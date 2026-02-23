#include "Renderer.h"

#include "Loopie/Core/Assert.h"
#include "Loopie/Components/Transform.h"
#include "Loopie/Render/Gizmo.h"
#include <iostream>

#include <glad/glad.h>
#include <IL/il.h>
#include <IL/ilu.h>

namespace Loopie {

	std::vector<Renderer::RenderItem> Renderer::s_RenderQueue = std::vector<Renderer::RenderItem>();
	std::vector<Camera*> Renderer::s_RenderCameras = std::vector<Camera*>();
	std::shared_ptr<UniformBuffer> Renderer::s_MatricesUniformBuffer = nullptr;
	bool Renderer::s_UseGizmos = true;
	vec4 Renderer::s_CurrentViewport = {0,0,0,0};

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
		s_UseGizmos = gizmo;
		s_MatricesUniformBuffer->SetData(&projectionMatrix[0][0], 0);
		s_MatricesUniformBuffer->SetData(&viewMatrix[0][0], 1);

		if(s_UseGizmos)
			Gizmo::BeginGizmo();
	}

	void Renderer::EndScene()
	{
		FlushRenderQueue();
		Gizmo::EndGizmo();
	}

	void Renderer::AddRenderItem(std::shared_ptr<VertexArray> vao, std::shared_ptr<Material> material, const Transform* transform, const std::vector<matrix4>& bones)
	{
		s_RenderQueue.emplace_back(RenderItem{ vao, vao->GetIndexBuffer().GetCount(), material, transform, bones});
	}

	void Renderer::FlushRenderItem(std::shared_ptr<VertexArray> vao, std::shared_ptr<Material> material, const Transform* transform, const std::vector<matrix4>& bones)
	{
		FlushRenderItem(vao, material, transform->GetLocalToWorldMatrix(), bones);
	}

	void Renderer::FlushRenderItem(std::shared_ptr<VertexArray> vao, std::shared_ptr<Material> material, const matrix4& modelMatrix, const std::vector<matrix4>& bones)
	{
		vao->Bind();
		material->Bind();
		SetRenderUniforms(material, modelMatrix, bones);
		glDrawElements(GL_TRIANGLES, vao->GetIndexBuffer().GetCount(), GL_UNSIGNED_INT, nullptr);
		vao->Unbind();
	}

	void Renderer::FlushRenderQueue()
	{
		/// SORT By Material


		///

		for (const RenderItem& item : s_RenderQueue) {
			
			item.VAO->Bind();
			item.Material->Bind();
			SetRenderUniforms(item.Material, item.Transform);
			glDrawElements(GL_TRIANGLES, item.IndexCount, GL_UNSIGNED_INT, nullptr);
			item.VAO->Unbind();
		}

		s_RenderQueue.clear();
	}

	void Renderer::SetRenderUniforms(std::shared_ptr<Material> material, const Transform* transform, const std::vector<matrix4>& bones)
	{
		SetRenderUniforms(material, transform->GetLocalToWorldMatrix(), bones);
	}
	void Renderer::SetRenderUniforms(std::shared_ptr<Material> material, const matrix4& modelMatrix, const std::vector<matrix4>& bones)
	{
		material->GetShader().SetUniformMat4("lp_Transform", modelMatrix);

		if (!bones.empty())
		{
			size_t count = std::min(bones.size(), size_t(100));
			material->GetShader().SetUniformMat4Array("lp_Bones", bones.data(), count);
		}

	}
	void Renderer::EnableDepth()
	{
			glEnable(GL_DEPTH_TEST);
	}
	void Renderer::DisableDepth()
	{
			glDisable(GL_DEPTH_TEST);
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
}
