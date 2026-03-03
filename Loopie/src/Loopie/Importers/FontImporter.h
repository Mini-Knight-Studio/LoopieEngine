#pragma once
#include "Loopie/Resources/Types/Font.h"
#include "Loopie/Resources/MetadataRegistry.h"

#include <string>
#include <vector>

namespace Loopie
{
	struct FontImportSettings
	{
		int pixelSize = 48;
		int firstChar = 32;
		int lastChar = 126;

		int padding = 1;
		int maxAtlasSize = 4096;
	};

	struct FontCacheHeader
	{
		uint32_t magic = 0x4C50464E;
		uint32_t version = 1;

		int32_t atlasWidth = 0;
		int32_t atlasHeight = 0;
		int32_t pixelSize = 0;

		int32_t ascender = 0;
		int32_t descender = 0;
		int32_t lineHeight = 0;

		uint32_t glyphCount = 0;

		int32_t compressedSize = 0;
		int32_t uncompressedSize = 0;
	};

	struct PackedGlyph
	{
		int32_t codepoint = 0;

		int32_t sizeX = 0;
		int32_t sizeY = 0;

		int32_t bearingX = 0;
		int32_t bearingY = 0;

		uint32_t advance = 0;

		float uvMinX = 0.0f;
		float uvMinY = 0.0f;
		float uvMaxX = 0.0f;
		float uvMaxY = 0.0f;
	};

	struct GlyphBitmap
	{
		int codepoint = 0;
		int w = 0;
		int h = 0;
		int bearingX = 0;
		int bearingY = 0;
		unsigned int advance = 0;

		std::vector<unsigned char> alpha;
	};

	struct PlacedGlyph
	{
		GlyphBitmap* src = nullptr;
		int x = 0;
		int y = 0;
	};

	class FontImporter
	{
	public:
		static void ImportFont(const std::string& filepath, Metadata& metadata, const FontImportSettings& settings = FontImportSettings());
		static void LoadFont(const std::string& cachePath, Font& font);
		static bool CheckIfIsFont(const char* path);

	private:
		static void WriteRgba(unsigned char* dst, unsigned char a);
		static std::string ToLower(std::string s);
	};
}