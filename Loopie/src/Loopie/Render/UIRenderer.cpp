#include "UIRenderer.h"

#include "Loopie/Render/VertexBuffer.h"
#include "Loopie/Render/IndexBuffer.h"
#include "Loopie/Resources/AssetRegistry.h"
#include "Loopie/Resources/ResourceManager.h"
#include "Loopie/Render/Renderer.h"

#include "Loopie/Importers/MaterialImporter.h"
#include "Loopie/Core/Log.h"

#include "glad/glad.h"

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
			0.0f, 0.0f, 0.0f,     0.0f, 1.0f,
			1.0f, 0.0f, 0.0f,     1.0f, 1.0f,
			1.0f, 1.0f, 0.0f,     1.0f, 0.0f,
			0.0f, 1.0f, 0.0f,     0.0f, 0.0f,
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
		if (!meta.HasCache)
		{
			MaterialImporter::ImportMaterial(uiMatPath, meta);
		}
		s_material = ResourceManager::GetMaterial(meta);
		s_material->Load();
		s_material->SetIfEditable(true);
		s_material->SetTextureOwnership(false);

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

	float UIRenderer::AlignFactor(TextHorizontalAlignment alignment)
	{
		switch (alignment)
		{
		case TextHorizontalAlignment::Left:
			return 0.0f;
		case TextHorizontalAlignment::Center:
			return 0.5f;
		case TextHorizontalAlignment::Right:
			return 1.0f;
		default:
			return 0.0f;
		}
	}

	float UIRenderer::AlignFactor(TextVerticalAlignment alignment)
	{
		switch (alignment)
		{
		case TextVerticalAlignment::Top:
			return 0.0f;
		case TextVerticalAlignment::Middle:
			return 0.5f;
		case TextVerticalAlignment::Bottom:
			return 1.0f;
		default:
			return 0.0f;
		}
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

		s_material->SetTexture("u_Albedo", texture);

		Renderer::FlushRenderItem(s_quadVAO, s_material, model);
	}

	void UIRenderer::DrawImageWorld(const matrix4& modelMatrix, const std::shared_ptr<Texture>& texture, const vec4& tint)
	{
		EnsureInit();
		if (!s_quadVAO || !s_material || !texture)
			return;

		UniformValue c;
		c.type = UniformType_vec4;
		c.value = tint;
		s_material->SetShaderVariable("u_Color", c);
		
		s_material->SetTexture("u_Albedo",texture);
		
		Renderer::FlushRenderItem(s_quadVAO, s_material, modelMatrix);
	}

	void UIRenderer::DrawText(const vec2& posPixels, const vec2& sizePixels, const std::string& text, const std::shared_ptr<Font>& font, const vec4& color, float scale,
							  TextSizeMode sizeMode, float fontSize, TextHorizontalAlignment hAlign, TextVerticalAlignment vAlign)
	{
		EnsureInit();

		if (!s_quadVAO || !s_material || !font || font->GetRendererId() == 0 || text.empty())
			return;

		float x = 0.0f;
		float y = 0.0f;

		float minX = 0.0f;
		float minY = 0.0f;
		float maxX = 0.0f;
		float maxY = 0.0f;

		const float baseScale = (scale <= 0.0f) ? 1.0f : scale;

		float fontScale = baseScale;
		if (sizeMode == TextSizeMode::FixedSize)
		{
			const float px = (fontSize <= 0.0f) ? (float)font->GetPixelSize() : fontSize;
			const float denom = (float)std::max(1, font->GetPixelSize());
			fontScale = (px / denom) * baseScale;
		}

		for (size_t i = 0; i < text.size(); i++)
		{
			const unsigned char ch = (unsigned char)text[i];

			if (ch == '\n')
			{
				x = 0.0f;
				y -= (float)font->GetLineHeight() * fontScale;
				continue;
			}

			const FontGlyph* g = font->GetGlyph((int)ch);
			if (!g)
				continue;

			const float xpos = x + (float)g->bearing.x * fontScale;
			const float ypos = y - ((float)g->size.y - (float)g->bearing.y) * fontScale;
			const float w = (float)g->size.x * fontScale;
			const float h = (float)g->size.y * fontScale;

			minX = std::min(minX, xpos);
			minY = std::min(minY, ypos);
			maxX = std::max(maxX, xpos + w);
			maxY = std::max(maxY, ypos + h);

			x += ((float)g->advance / 64.0f) * fontScale;
		}

		const float textW = std::max(1.0f, maxX - minX);
		const float textH = std::max(1.0f, maxY - minY);

		float fitScale = 1.0f;
		if (sizeMode == TextSizeMode::AutoSize)
		{
			if (sizePixels.x > 0.0f && sizePixels.y > 0.0f)
			{
				const float sx = sizePixels.x / textW;
				const float sy = sizePixels.y / textH;
				fitScale = std::min(sx, sy);
			}
		}

		UniformValue c;
		c.type = UniformType_vec4;
		c.value = color;
		s_material->SetShaderVariable("u_Color", c);
		s_material->SetTextureBufferOverride(font->GetAtlasTextureBuffer());

		const float contentW = textW * fitScale;
		const float contentH = textH * fitScale;

		const float ax = AlignFactor(hAlign);
		const float ay = AlignFactor(vAlign);

		const float alignOffsetX = ax * (sizePixels.x - contentW);
		const float alignOffsetY = ay * (sizePixels.y - contentH);

		const float ox = (- minX * fitScale) + alignOffsetX;
		const float oy = (- minY * fitScale) + alignOffsetY;

		x = 0.0f;
		y = 0.0f;

		for (size_t i = 0; i < text.size(); i++)
		{
			const unsigned char ch = (unsigned char)text[i];

			if (ch == '\n')
			{
				x = 0.0f;
				y -= (float)font->GetLineHeight() * fontScale;
				continue;
			}

			const FontGlyph* g = font->GetGlyph((int)ch);
			if (!g)
				continue;

			const float xpos = posPixels.x + (x + (float)g->bearing.x * fontScale) * fitScale + ox;
			const float ypos = posPixels.y + (y - ((float)g->size.y - (float)g->bearing.y) * fontScale) * fitScale + oy;

			const float w = (float)g->size.x * fontScale * fitScale;
			const float h = (float)g->size.y * fontScale * fitScale;

			UniformValue uv;
			uv.type = UniformType_vec4;
			uv.value = vec4(g->uvMin.x, g->uvMin.y, g->uvMax.x, g->uvMax.y);
			s_material->SetShaderVariable("u_UVRect", uv);

			matrix4 model(1.0f);
			model = glm::translate(model, vec3(xpos, ypos, 0.0f));
			model = glm::scale(model, vec3(w, h, 1.0f));

			Renderer::FlushRenderItem(s_quadVAO, s_material, model);

			x += ((float)g->advance / 64.0f) * fontScale;
		}

		UniformValue uvReset;
		uvReset.type = UniformType_vec4;
		uvReset.value = vec4(0.0f, 0.0f, 1.0f, 1.0f);
		s_material->SetShaderVariable("u_UVRect", uvReset);
		s_material->ClearTextureBufferOverride();
	}

	void UIRenderer::DrawTextWorld(const matrix4& modelMatrix, const vec2& sizePixels, const std::string& text, const std::shared_ptr<Font>& font, const vec4& color, float scale,
								   TextSizeMode sizeMode, float fontSize, TextHorizontalAlignment hAlign, TextVerticalAlignment vAlign)
	{
		EnsureInit();

		if (!s_quadVAO || !s_material || !font || font->GetRendererId() == 0 || text.empty())
			return;

		float x = 0.0f;
		float y = 0.0f;

		float minX = 0.0f;
		float minY = 0.0f;
		float maxX = 0.0f;
		float maxY = 0.0f;

		const float baseScale = (scale <= 0.0f) ? 1.0f : scale;

		float fontScale = baseScale;
		if (sizeMode == TextSizeMode::FixedSize)
		{
			const float px = (fontSize <= 0.0f) ? (float)font->GetPixelSize() : fontSize;
			const float denom = (float)std::max(1, font->GetPixelSize());
			fontScale = (px / denom) * baseScale;
		}

		for (size_t i = 0; i < text.size(); i++)
		{
			const unsigned char ch = (unsigned char)text[i];

			if (ch == '\n')
			{
				x = 0.0f;
				y -= (float)font->GetLineHeight() * fontScale;
				continue;
			}

			const FontGlyph* g = font->GetGlyph((int)ch);
			if (!g)
				continue;

			const float xpos = x + (float)g->bearing.x * fontScale;
			const float ypos = y - ((float)g->size.y - (float)g->bearing.y) * fontScale;
			const float w = (float)g->size.x * fontScale;
			const float h = (float)g->size.y * fontScale;

			minX = std::min(minX, xpos);
			minY = std::min(minY, ypos);
			maxX = std::max(maxX, xpos + w);
			maxY = std::max(maxY, ypos + h);

			x += ((float)g->advance / 64.0f) * fontScale;
		}

		const float textW = std::max(1.0f, maxX - minX);
		const float textH = std::max(1.0f, maxY - minY);

		float fitScale = 1.0f;
		if (sizeMode == TextSizeMode::AutoSize)
		{
			if (sizePixels.x > 0.0f && sizePixels.y > 0.0f)
			{
				const float sx = sizePixels.x / textW;
				const float sy = sizePixels.y / textH;
				fitScale = std::min(sx, sy);
			}
		}

		UniformValue c;
		c.type = UniformType_vec4;
		c.value = color;
		s_material->SetShaderVariable("u_Color", c);
		s_material->SetTextureBufferOverride(font->GetAtlasTextureBuffer());

		const float contentW = textW * fitScale;
		const float contentH = textH * fitScale;

		const float ax = AlignFactor(hAlign);
		const float ay = AlignFactor(vAlign);

		const float alignOffsetX = ax * (sizePixels.x - contentW);
		const float alignOffsetY = ay * (sizePixels.y - contentH);

		const float ox = (-minX * fitScale) + alignOffsetX;
		const float oy = (-minY * fitScale) + alignOffsetY;

		x = 0.0f;
		y = 0.0f;

		for (size_t i = 0; i < text.size(); i++)
		{
			const unsigned char ch = (unsigned char)text[i];

			if (ch == '\n')
			{
				x = 0.0f;
				y -= (float)font->GetLineHeight() * fontScale;
				continue;
			}

			const FontGlyph* g = font->GetGlyph((int)ch);
			if (!g)
				continue;

			const float xpos = (x + (float)g->bearing.x * fontScale) * fitScale + ox;
			const float ypos = (y - ((float)g->size.y - (float)g->bearing.y) * fontScale) * fitScale + oy;

			const float w = (float)g->size.x * fontScale * fitScale;
			const float h = (float)g->size.y * fontScale * fitScale;

			UniformValue uv;
			uv.type = UniformType_vec4;
			uv.value = vec4(g->uvMin.x, g->uvMin.y, g->uvMax.x, g->uvMax.y);
			s_material->SetShaderVariable("u_UVRect", uv);

			matrix4 glyphLocal(1.0f);
			glyphLocal = glm::translate(glyphLocal, vec3(xpos, ypos, 0.0f));
			glyphLocal = glm::scale(glyphLocal, vec3(w, h, 1.0f));

			Renderer::FlushRenderItem(s_quadVAO, s_material, modelMatrix * glyphLocal);

			x += ((float)g->advance / 64.0f) * fontScale;
		}

		UniformValue uvReset;
		uvReset.type = UniformType_vec4;
		uvReset.value = vec4(0.0f, 0.0f, 1.0f, 1.0f);
		s_material->SetShaderVariable("u_UVRect", uvReset);
		s_material->ClearTextureBufferOverride();
	}

	void UIRenderer::EnsureInit()
	{
		if (!s_initialized)
			Init();
	}
}