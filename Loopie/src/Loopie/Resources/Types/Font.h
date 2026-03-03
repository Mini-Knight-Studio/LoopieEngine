#pragma once
#include "Loopie/Resources/Resource.h"
#include "Loopie/Math/MathTypes.h"
#include "Loopie/Render/TextureBuffer.h"

#include <memory>
#include <unordered_map>

namespace Loopie
{
	struct FontGlyph
	{
		int Codepoint = 0;

		ivec2 size = ivec2(0);
		ivec2 bearing = ivec2(0);
		unsigned int advance = 0;

		vec2 uvMin = vec2(0.0f);
		vec2 uvMax = vec2(0.0f);
	};

	class Font : public Resource
	{
		friend class FontImporter;

	public:
		DEFINE_TYPE(Font)

		Font(const UUID& id);
		~Font() = default;

		bool Load() override;

		const FontGlyph* GetGlyph(int codepoint) const;

		unsigned int GetRendererId() const { return m_tb ? m_tb->GetRendererID() : 0; }
		ivec2 GetAtlasSize() const { return ivec2(m_atlasWidth, m_atlasHeight); }

		int GetPixelSize() const { return m_pixelSize; }
		int GetAscender() const { return m_ascender; }
		int GetDescender() const { return m_descender; }
		int GetLineHeight() const { return m_lineHeight; }

	private:
		int m_atlasWidth = 0;
		int m_atlasHeight = 0;
		int m_pixelSize = 0;

		int m_ascender = 0;
		int m_descender = 0;
		int m_lineHeight = 0;

		std::unordered_map<int, FontGlyph> m_glyphs;
		std::shared_ptr<TextureBuffer> m_tb;
	};
}