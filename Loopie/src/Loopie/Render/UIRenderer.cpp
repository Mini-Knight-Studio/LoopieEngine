#include "UIRenderer.h"

#include "Loopie/Render/VertexBuffer.h"
#include "Loopie/Render/IndexBuffer.h"
#include "Loopie/Resources/AssetRegistry.h"
#include "Loopie/Resources/ResourceManager.h"
#include "Loopie/Render/Renderer.h"

#include "Loopie/Importers/MaterialImporter.h"
#include "Loopie/Core/Log.h"

#include "glad/glad.h"

#include <algorithm>

namespace Loopie
{
	bool UIRenderer::s_initialized = false;
	std::shared_ptr<VertexArray> UIRenderer::s_quadVAO = nullptr;
	std::shared_ptr<VertexBuffer> UIRenderer::s_quadVBO = nullptr;
	std::shared_ptr<IndexBuffer> UIRenderer::s_quadEBO = nullptr;
	std::shared_ptr<Material> UIRenderer::s_material = nullptr;
	Shader *UIRenderer::s_shader = nullptr;

	static std::vector<std::string> SplitLines(const std::string &text)
	{
		std::vector<std::string> lines;

		size_t start = 0;
		size_t end = text.find('\n');

		while (end != std::string::npos)
		{
			lines.push_back(text.substr(start, end - start));
			start = end + 1;
			end = text.find('\n', start);
		}

		lines.push_back(text.substr(start));

		return lines;
	}

	void UIRenderer::Init()
	{
		if (s_initialized)
			return;

		const float vertices[] =
			{
				// pos                // uv
				0.0f,
				0.0f,
				0.0f,
				0.0f,
				1.0f,
				1.0f,
				0.0f,
				0.0f,
				1.0f,
				1.0f,
				1.0f,
				1.0f,
				0.0f,
				1.0f,
				0.0f,
				0.0f,
				1.0f,
				0.0f,
				0.0f,
				0.0f,
			};

		const unsigned int indices[] = {0, 1, 2, 2, 3, 0};

		s_quadVBO = std::make_shared<VertexBuffer>(vertices, (unsigned int)sizeof(vertices));
		BufferLayout &layout = s_quadVBO->GetLayout();
		layout.AddLayoutElement(0, GLVariableType::FLOAT, 3, "a_Position");
		layout.AddLayoutElement(1, GLVariableType::FLOAT, 2, "a_TexCoord");

		s_quadEBO = std::make_shared<IndexBuffer>(indices, (unsigned int)(sizeof(indices) / sizeof(indices[0])));

		s_quadVAO = std::make_shared<VertexArray>();
		s_quadVAO->AddBuffer(s_quadVBO.get(), s_quadEBO.get());

		const char *uiMatPath = "assets\\materials\\ui_default.mat";
		Metadata &meta = AssetRegistry::GetOrCreateMetadata(uiMatPath);
		if (!meta.HasCache)
		{
			MaterialImporter::ImportMaterial(uiMatPath, meta);
		}
		s_material = ResourceManager::GetMaterial(meta);
		s_material->Load();
		s_material->SetIfEditable(true);
		s_material->SetTextureOwnership(false);

		s_shader = new Shader("assets\\shaders\\UIQuad.shader");
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

	bool UIRenderer::IsSpaceExceptNewline(char c)
	{
		return c == ' ' || c == '\t' || c == '\v' || c == '\f' || c == '\r';
	}

	float UIRenderer::MeasureCharAdvance(const std::shared_ptr<Font> &font, unsigned char ch, float fontScale, float spaceAdvance)
	{
		if (ch == '\t')
			return spaceAdvance * 4.0f;
		if (ch == ' ')
			return spaceAdvance;

		const FontGlyph *g = font ? font->GetGlyph((int)ch) : nullptr;
		if (!g)
			return 0.0f;
		return ((float)g->advance / 64.0f) * fontScale;
	}

	float UIRenderer::MeasureStringAdvance(const std::shared_ptr<Font> &font, const std::string &s, float fontScale, float spaceAdvance, float letterSpacing)
	{
		float width = 0.0f;
		bool first = true;
		for (size_t i = 0; i < s.size(); ++i)
		{
			if (!first)
				width += letterSpacing * fontScale;
			width += MeasureCharAdvance(font, (unsigned char)s[i], fontScale, spaceAdvance);
			first = false;
		}
		return width;
	}

	std::string UIRenderer::WrapTextToWidth(const std::string &text, const std::shared_ptr<Font> &font, float fontScale,
											float maxWidth, TextWrapMode wrapMode, float letterSpacing, float wordSpacing)
	{
		if (wrapMode == TextWrapMode::NoWrap)
			return text;
		if (!font || text.empty() || maxWidth <= 0.0f)
			return text;

		float spaceAdvance = 4.0f * fontScale;
		if (const FontGlyph *spaceGlyph = font->GetGlyph((int)' '))
			spaceAdvance = std::max(0.0f, ((float)spaceGlyph->advance / 64.0f) * fontScale);
		if (spaceAdvance <= 0.0f)
			spaceAdvance = 4.0f * fontScale;

		std::string out;
		out.reserve(text.size() + 16);
		std::string line;
		line.reserve(128);
		float lineW = 0.0f;
		bool pendingSpace = false;
		const float wordAdvance = spaceAdvance + (wordSpacing * fontScale);

		auto flushLine = [&]()
		{
			out += line;
			line.clear();
			lineW = 0.0f;
			pendingSpace = false;
		};

		auto appendWord = [&](const std::string &word)
		{
			if (word.empty())
				return;

			const float wordW = MeasureStringAdvance(font, word, fontScale, spaceAdvance, letterSpacing);
			const float spaceW = (!line.empty() && pendingSpace) ? wordAdvance : 0.0f;
			const float required = wordW + spaceW;

			const bool fitsCurrentLine = line.empty() || (lineW + required <= maxWidth);
			if (fitsCurrentLine)
			{
				if (!line.empty() && pendingSpace)
				{
					line += ' ';
					lineW += wordAdvance;
				}
				pendingSpace = false;
				line += word;
				lineW += wordW;
				return;
			}

			flushLine();
			out += '\n';

			if (wordW <= maxWidth)
			{
				line = word;
				lineW = wordW;
				return;
			}

			std::string segment;
			segment.reserve(word.size());
			float segW = 0.0f;
			for (size_t i = 0; i < word.size(); ++i)
			{
				const unsigned char ch = (unsigned char)word[i];
				const float adv = MeasureCharAdvance(font, ch, fontScale, spaceAdvance);
				const float spacing = !segment.empty() ? (letterSpacing * fontScale) : 0.0f;
				if (!segment.empty() && (segW + spacing + adv > maxWidth))
				{
					out += segment;
					out += '\n';
					segment.clear();
					segW = 0.0f;
				}
				if (!segment.empty())
					segW += letterSpacing * fontScale;
				segment.push_back((char)ch);
				segW += adv;
			}

			line = segment;
			lineW = segW;
		};

		std::string word;
		word.reserve(32);
		for (size_t i = 0; i < text.size(); ++i)
		{
			const char c = text[i];
			if (c == '\r')
				continue;
			if (c == '\n')
			{
				appendWord(word);
				word.clear();
				flushLine();
				out += '\n';
				continue;
			}
			if (IsSpaceExceptNewline(c))
			{
				appendWord(word);
				word.clear();
				if (!line.empty())
					pendingSpace = true;
				continue;
			}

			word.push_back(c);
		}

		appendWord(word);
		out += line;
		return out;
	}

	void UIRenderer::DrawRect(const vec2 &posPixels, const vec2 &sizePixels, const vec4 &color)
	{
		EnsureInit();

		if (!s_quadVAO || !s_material)
			return;

		UniformValue uv;
		uv.type = UniformType_vec4;
		uv.value = vec4(0.0f, 0.0f, 1.0f, 1.0f);
		s_material->SetShaderVariable("u_UVRect", uv);

		matrix4 model(1.0f);
		model = glm::translate(model, vec3(posPixels.x, posPixels.y, 0.0f));
		model = glm::scale(model, vec3(sizePixels.x, sizePixels.y, 1.0f));

		UniformValue c;
		c.type = UniformType_vec4;
		c.value = color;
		s_material->SetShaderVariable("u_Color", c);

		Renderer::FlushRenderItem(s_quadVAO, s_material, model);
	}

	void UIRenderer::DrawImage(const vec2 &posPixels, const vec2 &sizePixels, const std::shared_ptr<Texture> &texture, const vec4 &tint)
	{
		DrawImage(posPixels, sizePixels, texture, tint, vec4(0.0f, 0.0f, 1.0f, 1.0f));
	}

	void UIRenderer::DrawImage(const vec2 &posPixels, const vec2 &sizePixels, const std::shared_ptr<Texture> &texture, const vec4 &tint, const vec4 &uvRect)
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

		UniformValue uv;
		uv.type = UniformType_vec4;
		uv.value = uvRect;
		s_material->SetShaderVariable("u_UVRect", uv);

		s_material->SetTexture("u_Albedo", texture);

		Renderer::FlushRenderItem(s_quadVAO, s_material, model);
	}

	void UIRenderer::DrawImageWorld(const matrix4 &modelMatrix, const std::shared_ptr<Texture> &texture, const vec4 &tint)
	{
		DrawImageWorld(modelMatrix, texture, tint, vec4(0.0f, 0.0f, 1.0f, 1.0f));
	}

	void UIRenderer::DrawImageWorld(const matrix4 &modelMatrix, const std::shared_ptr<Texture> &texture, const vec4 &tint, const vec4 &uvRect)
	{
		EnsureInit();
		if (!s_quadVAO || !s_material || !texture)
			return;

		UniformValue c;
		c.type = UniformType_vec4;
		c.value = tint;
		s_material->SetShaderVariable("u_Color", c);

		UniformValue uv;
		uv.type = UniformType_vec4;
		uv.value = uvRect;
		s_material->SetShaderVariable("u_UVRect", uv);

		s_material->SetTexture("u_Albedo", texture);

		Renderer::FlushRenderItem(s_quadVAO, s_material, modelMatrix);
	}

	void UIRenderer::DrawTextContainer(const vec2 &posPixels, const vec2 &sizePixels, const std::string &text, const std::shared_ptr<Font> &font, const vec4 &color, float scale,
									   TextSizeMode sizeMode, float fontSize, TextHorizontalAlignment hAlign, TextVerticalAlignment vAlign, TextWrapMode wrapMode,
									   float lineSpacing, float wordSpacing, float letterSpacing, int visibleCharacters)
	{
		EnsureInit();

		if (!s_quadVAO || !s_material || !font || text.empty())
			return;

		if (font->GetRendererId() == 0)
			font->Load();

		auto atlasTB = font->GetAtlasTextureBuffer();
		if (!atlasTB || atlasTB->GetRendererID() == 0)
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

		const TextWrapMode effectiveWrap =
			(sizeMode == TextSizeMode::AutoSize)
				? TextWrapMode::NoWrap
				: wrapMode;

		const std::string renderText =
			WrapTextToWidth(text, font, fontScale, sizePixels.x, effectiveWrap, letterSpacing, wordSpacing);
		if (renderText.empty())
			return;

		const float lineAdvance = ((float)font->GetLineHeight() + lineSpacing) * fontScale;
		const float letterAdvance = letterSpacing * fontScale;
		const FontGlyph *spaceGlyph = font->GetGlyph((int)' ');
		const float baseSpaceAdvance = spaceGlyph ? ((float)spaceGlyph->advance / 64.0f) * fontScale : 4.0f * fontScale;
		const float wordAdvance = baseSpaceAdvance + (wordSpacing * fontScale);
		bool lineStart = true;
		bool lastWasSpace = true;

		const auto lines = SplitLines(renderText);

		std::vector<float> lineWidths;
		lineWidths.reserve(lines.size());

		float maxLineWidth = 0.0f;

		for (const auto &line : lines)
		{
			float w = MeasureStringAdvance(
				font,
				line,
				fontScale,
				wordAdvance,
				letterSpacing);

			lineWidths.push_back(w);
			maxLineWidth = std::max(maxLineWidth, w);
		}

		int visibleCount =
			(visibleCharacters < 0)
				? (int)renderText.size()
				: visibleCharacters;

		int drawn = 0;

		for (size_t i = 0; i < renderText.size(); i++)
		{
			const unsigned char ch = (unsigned char)renderText[i];

			if (ch == '\n')
			{
				x = 0.0f;
				y -= lineAdvance;
				lineStart = true;
				lastWasSpace = true;
				continue;
			}

			if (ch == '\t' || ch == ' ')
			{
				x += (ch == '\t') ? wordAdvance * 4.0f : wordAdvance;
				lineStart = false;
				lastWasSpace = true;
				continue;
			}

			if (!lineStart && !lastWasSpace)
				x += letterAdvance;

			const FontGlyph *g = font->GetGlyph((int)ch);
			if (!g)
				continue;

			if (drawn >= visibleCount)
			{
				x += ((float)g->advance / 64.0f) * fontScale;
				lineStart = false;
				lastWasSpace = false;
				continue;
			}

			const float xpos = x + (float)g->bearing.x * fontScale;
			const float ypos = y - ((float)g->size.y - (float)g->bearing.y) * fontScale;
			const float w = (float)g->size.x * fontScale;
			const float h = (float)g->size.y * fontScale;

			minX = std::min(minX, xpos);
			minY = std::min(minY, ypos);
			maxX = std::max(maxX, xpos + w);
			maxY = std::max(maxY, ypos + h);

			x += ((float)g->advance / 64.0f) * fontScale;
			lineStart = false;
			lastWasSpace = false;
		}

		const float textW = std::max(1.0f, maxLineWidth);
		int lineCount = 1;
		for (char c : renderText)
		{
			if (c == '\n')
				lineCount++;
		}

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
		s_material->SetTextureBufferOverride(atlasTB);

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

		lineStart = true;
		lastWasSpace = true;

		size_t currentLine = 0;

		float lineOffsetX =
			(textW - lineWidths[currentLine]) * ax;

		for (size_t i = 0; i < renderText.size(); i++)
		{
			const unsigned char ch = (unsigned char)renderText[i];

			if (ch == '\n')
			{
				x = 0.0f;
				y -= lineAdvance;
				lineStart = true;
				lastWasSpace = true;
				continue;
			}

			if (ch == '\t' || ch == ' ')
			{
				x += (ch == '\t') ? wordAdvance * 4.0f : wordAdvance;
				lineStart = false;
				lastWasSpace = true;
				continue;
			}

			if (!lineStart && !lastWasSpace)
				x += letterAdvance;

			const FontGlyph *g = font->GetGlyph((int)ch);
			if (!g)
				continue;

			if (drawn >= visibleCount)
			{
				x += ((float)g->advance / 64.0f) * fontScale;
				lineStart = false;
				lastWasSpace = false;
				continue;
			}

			const float xpos = posPixels.x +
							   (x + (float)g->bearing.x * fontScale + lineOffsetX) * fitScale +
							   ox;
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

			drawn++;
			lineStart = false;
			lastWasSpace = false;
		}

		UniformValue uvReset;
		uvReset.type = UniformType_vec4;
		uvReset.value = vec4(0.0f, 0.0f, 1.0f, 1.0f);
		s_material->SetShaderVariable("u_UVRect", uvReset);
		s_material->ClearTextureBufferOverride();
	}

	void UIRenderer::DrawTextWorld(const matrix4 &modelMatrix, const vec2 &sizePixels, const std::string &text, const std::shared_ptr<Font> &font, const vec4 &color, float scale,
								   TextSizeMode sizeMode, float fontSize, TextHorizontalAlignment hAlign, TextVerticalAlignment vAlign, TextWrapMode wrapMode,
								   float lineSpacing, float wordSpacing, float letterSpacing, int visibleCharacters)
	{
		EnsureInit();

		if (!s_quadVAO || !s_material || !font || text.empty())
			return;

		if (font->GetRendererId() == 0)
			font->Load();

		auto atlasTB = font->GetAtlasTextureBuffer();
		if (!atlasTB || atlasTB->GetRendererID() == 0)
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

		const std::string renderText = WrapTextToWidth(text, font, fontScale, sizePixels.x, wrapMode, letterSpacing, wordSpacing);
		if (renderText.empty())
			return;

		const float lineAdvance = ((float)font->GetLineHeight() + lineSpacing) * fontScale;
		const float letterAdvance = letterSpacing * fontScale;
		const FontGlyph *spaceGlyph = font->GetGlyph((int)' ');
		const float baseSpaceAdvance = spaceGlyph ? ((float)spaceGlyph->advance / 64.0f) * fontScale : 4.0f * fontScale;
		const float wordAdvance = baseSpaceAdvance + (wordSpacing * fontScale);
		bool lineStart = true;
		bool lastWasSpace = true;

		const auto lines = SplitLines(renderText);

		std::vector<float> lineWidths;
		lineWidths.reserve(lines.size());

		float maxLineWidth = 0.0f;

		for (const auto &line : lines)
		{
			float w = MeasureStringAdvance(
				font,
				line,
				fontScale,
				wordAdvance,
				letterSpacing);

			lineWidths.push_back(w);
			maxLineWidth = std::max(maxLineWidth, w);
		}

		int visibleCount =
			(visibleCharacters < 0)
				? (int)renderText.size()
				: visibleCharacters;

		int drawn = 0;

		for (size_t i = 0; i < renderText.size(); i++)
		{
			const unsigned char ch = (unsigned char)renderText[i];

			if (ch == '\n')
			{
				x = 0.0f;
				y -= lineAdvance;
				lineStart = true;
				lastWasSpace = true;
				continue;
			}

			if (ch == '\t' || ch == ' ')
			{
				x += (ch == '\t') ? wordAdvance * 4.0f : wordAdvance;
				lineStart = false;
				lastWasSpace = true;
				continue;
			}

			if (!lineStart && !lastWasSpace)
				x += letterAdvance;

			const FontGlyph *g = font->GetGlyph((int)ch);
			if (!g)
				continue;

			if (drawn >= visibleCount)
			{
				x += ((float)g->advance / 64.0f) * fontScale;
				lineStart = false;
				lastWasSpace = false;
				continue;
			}

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

		const float textW = std::max(1.0f, maxLineWidth);
		int lineCount = 1;
		for (char c : renderText)
		{
			if (c == '\n')
				lineCount++;
		}

		const float textH =
			std::max(lineAdvance, lineCount * lineAdvance);

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
		s_material->SetTextureBufferOverride(atlasTB);

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

		lineStart = true;
		lastWasSpace = true;

		size_t currentLine = 0;

		float lineOffsetX =
			(textW - lineWidths[currentLine]) * ax;

		for (size_t i = 0; i < renderText.size(); i++)
		{
			const unsigned char ch = (unsigned char)renderText[i];

			if (ch == '\n')
			{
				x = 0.0f;
				y -= lineAdvance;
				lineStart = true;
				lastWasSpace = true;
				continue;
			}

			if (ch == '\t' || ch == ' ')
			{
				x += (ch == '\t') ? wordAdvance * 4.0f : wordAdvance;
				lineStart = false;
				lastWasSpace = true;
				continue;
			}

			if (!lineStart && !lastWasSpace)
				x += letterAdvance;

			const FontGlyph *g = font->GetGlyph((int)ch);
			if (!g)
				continue;

			if (drawn >= visibleCount)
			{
				x += ((float)g->advance / 64.0f) * fontScale;
				lineStart = false;
				lastWasSpace = false;
				continue;
			}

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

			drawn++;
			lineStart = false;
			lastWasSpace = false;
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