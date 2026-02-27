#include "UIRenderer.h"

#include "Loopie/Render/VertexBuffer.h"
#include "Loopie/Render/IndexBuffer.h"
#include "Loopie/Resources/AssetRegistry.h"
#include "Loopie/Resources/ResourceManager.h"
#include "Loopie/Render/Renderer.h"

#include "Loopie/Importers/MaterialImporter.h"
#include "Loopie/Core/Log.h"

namespace Loopie
{
	bool UIRenderer::s_initialized = false;
	std::shared_ptr<VertexArray> UIRenderer::s_quadVAO = nullptr;
	std::shared_ptr<VertexBuffer> UIRenderer::s_quadVBO = nullptr;
	std::shared_ptr<IndexBuffer> UIRenderer::s_quadEBO = nullptr;
	std::shared_ptr<Material> UIRenderer::s_material = nullptr;
	Shader* UIRenderer::s_shader = nullptr;
	
	void UIRenderer::Init()
	{
		if (s_initialized)
			return;

		const float vertices[] =
		{
			// pos                // uv
			0.0f, 0.0f, 0.0f,     0.0f, 0.0f,
			1.0f, 0.0f, 0.0f,     1.0f, 0.0f,
			1.0f, 1.0f, 0.0f,     1.0f, 1.0f,
			0.0f, 1.0f, 0.0f,     0.0f, 1.0f,
		};

		const unsigned int indices[] = { 0,1,2,2,3,0 };

		s_quadVBO = std::make_shared<VertexBuffer>(vertices, (unsigned int)sizeof(vertices));
		BufferLayout& layout = s_quadVBO->GetLayout();
		layout.AddLayoutElement(0, GLVariableType::FLOAT, 3, "a_Position");
		layout.AddLayoutElement(1, GLVariableType::FLOAT, 2, "a_TexCoord");

		s_quadEBO = std::make_shared<IndexBuffer>(indices, (unsigned int)(sizeof(indices) / sizeof(indices[0])));

		s_quadVAO = std::make_shared<VertexArray>();
		s_quadVAO->AddBuffer(s_quadVBO.get(), s_quadEBO.get());

		const char* uiMatPath = "assets/materials/ui_default.mat";
		Metadata& meta = AssetRegistry::GetOrCreateMetadata(uiMatPath);
		s_material = ResourceManager::GetMaterial(meta);

		MaterialImporter::ImportMaterial(uiMatPath, meta);
		if (!s_material)
		{
			s_material = Material::GetDefault();
		}
		else
		{
			s_material->Load();
		}

		s_shader = new Shader("assets/shaders/UIQuad.shader");
		s_material->SetShader(*s_shader);
		s_initialized = true;

	}

	void UIRenderer::Shutdown()
	{
		delete s_shader;
		s_shader = nullptr;

		s_quadVAO.reset();
		s_quadVBO.reset();
		s_quadEBO.reset();
		s_material.reset();
		s_initialized = false;
	}

	void UIRenderer::DrawRect(const vec2& posPixels, const vec2& sizePixels, const vec4& color)
	{
		EnsureInit();

		if(!s_quadVAO || !s_material)
			return;

		matrix4 model(1.0f);
		model = glm::translate(model, vec3(posPixels.x,posPixels.y, 0.0f));
		model = glm::scale(model, vec3(sizePixels.x, sizePixels.y, 1.0f));

		UniformValue c;
		c.type = UniformType_vec4;
		c.value = color;
		s_material->SetShaderVariable("u_Color", c);

		Renderer::FlushRenderItem(s_quadVAO, s_material, model);
	}

	void UIRenderer::DrawImage(const vec2& posPixels, const vec2& sizePixels, const std::shared_ptr<Texture>& texture, const vec4& tint)
	{
		EnsureInit();

		if (!s_quadVAO || !s_material || !texture)
			return;

		matrix4 model(1.0f);
		model = glm::translate(model, vec3(posPixels.x, posPixels.y, 0.0f));
		model = glm::scale(model, vec3(sizePixels.x, sizePixels.y, 1.0f));

		UniformValue c;
		c.type = UniformType_vec4;
		c.value = tint;
		s_material->SetShaderVariable("u_Color", c);

		s_material->SetTexture(texture);

		Renderer::FlushRenderItem(s_quadVAO, s_material, model);
	}

	void UIRenderer::EnsureInit()
	{
		if (!s_initialized)
			Init();
	}
}