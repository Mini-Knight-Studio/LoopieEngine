#include "FontImporter.h"

#include "Loopie/Core/Log.h"
#include "Loopie/Core/Application.h"
#include "Loopie/Math/MathUtils.h"

#include <filesystem>
#include <fstream>
#include <algorithm>
#include <cstdint>

#include <lz4.h>

#include <ft2build.h>
#include FT_FREETYPE_H

void Loopie::FontImporter::ImportFont(const std::string& filepath, Metadata& metadata, const FontImportSettings& settings)
{
	if (metadata.HasCache && !metadata.IsOutdated)
		return;

	if (!std::filesystem::exists(filepath))
	{
		Log::Error("Font file '{0}' does not exist.", filepath);
		return;
	}

	FT_Library ft = nullptr;
	if (FT_Init_FreeType(&ft))
	{
		Log::Error("Failed to initialize FreeType library.");
		return;
	}

	FT_Face face = nullptr;
	if (FT_New_Face(ft, filepath.c_str(), 0, &face))
	{
		Log::Error("Failed to load font '{0}'.", filepath);
		FT_Done_FreeType(ft);
		return;
	}

	FT_Set_Pixel_Sizes(face, 0, (FT_UInt)settings.pixelSize);

	const int firstChar = settings.firstChar;
	const int lastChar = settings.lastChar;

	std::vector<GlyphBitmap> glyphs;
	glyphs.reserve((size_t)(lastChar - firstChar + 1));

	for (int c = firstChar; c <= lastChar; ++c)
	{
		if (FT_Load_Char(face, (FT_Long)c, FT_LOAD_RENDER))
			continue;

		FT_GlyphSlot g = face->glyph;

		GlyphBitmap gb;
		gb.codepoint = c;
		gb.w = (int)g->bitmap.width;
		gb.h = (int)g->bitmap.rows;
		gb.bearingX = (int)g->bitmap_left;
		gb.bearingY = (int)g->bitmap_top;
		gb.advance = (unsigned int)g->advance.x;

		gb.alpha.resize((size_t)(gb.w * gb.h));
		for (int y = 0; y < gb.h; ++y)
		{
			const unsigned char* row = g->bitmap.buffer + (size_t)y * (size_t)g->bitmap.pitch;
			memcpy(gb.alpha.data() + (size_t)y * (size_t)gb.w, row, (size_t)gb.w);
		}

		glyphs.push_back(std::move(gb));
	}

	int atlasWidth = 256;
	int atlasHeight = 256;

	std::vector<PlacedGlyph> placed;
	placed.reserve(glyphs.size());

	for (;;)
	{
		placed.clear();

		int penX = settings.padding;
		int penY = settings.padding;
		int rowH = 0;

		bool fits = true;

		for (auto& g : glyphs)
		{
			const int gw = g.w + settings.padding * 2;
			const int gh = g.h + settings.padding * 2;

			if (gw > atlasWidth || gh > atlasHeight)
			{
				fits = false;
				break;
			}

			if (penX + gw > atlasWidth)
			{
				penX = settings.padding;
				penY += rowH;
				rowH = 0;
			}

			if (penY + gh > atlasHeight)
			{
				fits = false;
				break;
			}

			PlacedGlyph pg;
			pg.src = &g;
			pg.x = penX + settings.padding;
			pg.y = penY + settings.padding;
			placed.push_back(pg);

			penX += gw;
			rowH = std::max(rowH, gh);
		}

		if (fits)
			break;

		if (atlasWidth < settings.maxAtlasSize)
			atlasWidth *= 2;
		else if (atlasHeight < settings.maxAtlasSize)
			atlasHeight *= 2;
		else
		{
			Log::Error("Font atlas exceeded max size ({0}x{1}).", settings.maxAtlasSize, settings.maxAtlasSize);
			FT_Done_Face(face);
			FT_Done_FreeType(ft);
			return;
		}
	}

	atlasWidth = Math::nextPow2(atlasWidth);
	atlasHeight = Math::nextPow2(atlasHeight);

	const int channels = 4;
	const int uncompressedSize = atlasWidth * atlasHeight * channels;

	std::vector<unsigned char> rgba((size_t)uncompressedSize, 0);

	for (const auto& pg : placed)
	{
		const GlyphBitmap& g = *pg.src;

		for (int y = 0; y < g.h; ++y)
		{
			for (int x = 0; x < g.w; ++x)
			{
				unsigned char a = g.alpha[(size_t)y * (size_t)g.w + (size_t)x];
				
				const int dstX = pg.x + x;
				const int dstY = pg.y + y;

				unsigned char* dst = rgba.data() + ((size_t)dstY * (size_t)atlasWidth + (size_t)dstX) * 4;
				WriteRgba(dst, a);
			}
		}
	}

	const int maxCompressedSize = LZ4_compressBound(uncompressedSize);
	std::vector<char> compressed((size_t)maxCompressedSize);

	const int compressedSize = LZ4_compress_default(
		reinterpret_cast<const char*>(rgba.data()),
		compressed.data(),
		uncompressedSize,
		maxCompressedSize);

	if (compressedSize <= 0)
	{
		Log::Error("Font atlas compression failed: {0}", filepath);
		FT_Done_Face(face);
		FT_Done_FreeType(ft);
		return;
	}

	Project project = Application::GetInstance().m_activeProject;
	UUID id;

	std::filesystem::path locationPath = "Fonts";
	locationPath /= id.Get() + ".font";

	std::filesystem::path pathToWrite = project.GetChachePath() / locationPath;
	std::filesystem::create_directories(pathToWrite.parent_path());

	std::ofstream fs(pathToWrite, std::ios::binary | std::ios::trunc);
	if (!fs)
	{
		Log::Error("Failed to write font cache file: {0}", pathToWrite.string());
		FT_Done_Face(face);
		FT_Done_FreeType(ft);
		return;
	}

	FontCacheHeader header;
	header.atlasWidth = atlasWidth;
	header.atlasHeight = atlasHeight;
	header.pixelSize = settings.pixelSize;
	header.ascender = (int)(face->size->metrics.ascender >> 6);
	header.descender = (int)(face->size->metrics.descender >> 6);
	header.lineHeight = (int)(face->size->metrics.height >> 6);
	header.glyphCount = (uint32_t)placed.size();
	header.compressedSize = compressedSize;
	header.uncompressedSize = uncompressedSize;

	fs.write(reinterpret_cast<const char*>(&header), sizeof(header));

	for (const auto& pg : placed)
	{
		const GlyphBitmap& g = *pg.src;

		PackedGlyph pgd;
		pgd.codepoint = g.codepoint;
		pgd.sizeX = g.w;
		pgd.sizeY = g.h;
		pgd.bearingX = g.bearingX;
		pgd.bearingY = g.bearingY;
		pgd.advance = g.advance;

		pgd.uvMinX = (float)pg.x / (float)atlasWidth;
		pgd.uvMinY = (float)pg.y / (float)atlasHeight;
		pgd.uvMaxX = (float)(pg.x + g.w) / (float)atlasWidth;
		pgd.uvMaxY = (float)(pg.y + g.h) / (float)atlasHeight;
		
		fs.write(reinterpret_cast<const char*>(&pgd), sizeof(pgd));
	}

	fs.write(compressed.data(), compressedSize);
	fs.close();

	metadata.HasCache = true;
	metadata.CachesPath.clear();
	metadata.CachesPath.push_back(locationPath.string());
	metadata.Type = ResourceType::FONT;

	MetadataRegistry::SaveMetadata(filepath, metadata);

	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	Log::Trace("Font Imported -> {0} ({1}x{2}, RGBA8)", filepath, atlasWidth, atlasHeight);
}

void Loopie::FontImporter::LoadFont(const std::string& cachePath, Font& font)
{
	Project project = Application::GetInstance().m_activeProject;
	std::filesystem::path filepath = project.GetChachePath() / cachePath;

	if (!std::filesystem::exists(filepath))
	{
		Log::Warn("Font cache file '{0}' does not exist.", filepath.string());
		return;
	}

	std::ifstream file(filepath, std::ios::binary);
	if (!file)
	{
		Log::Warn("Error opening .font file -> {0}", filepath.string());
		return;
	}

	FontCacheHeader header{};
	file.read(reinterpret_cast<char*>(&header), sizeof(header));

	if (header.magic != 0x4C50464E || header.version != 1)
	{
		Log::Warn("Invalid font cache header -> {0}", filepath.string());
		return;
	}

	font.m_atlasWidth = header.atlasWidth;
	font.m_atlasHeight = header.atlasHeight;
	font.m_pixelSize = header.pixelSize;
	font.m_ascender = header.ascender;
	font.m_descender = header.descender;
	font.m_lineHeight = header.lineHeight;

	font.m_glyphs.clear();
	font.m_glyphs.reserve(header.glyphCount);

	for (uint32_t i = 0; i < header.glyphCount; ++i)
	{
		PackedGlyph pgd{};
		file.read(reinterpret_cast<char*>(&pgd), sizeof(pgd));

		FontGlyph fg;
		fg.Codepoint = pgd.codepoint;
		fg.size = ivec2(pgd.sizeX, pgd.sizeY);
		fg.bearing = ivec2(pgd.bearingX, pgd.bearingY);
		fg.advance = pgd.advance;
		fg.uvMin = vec2(pgd.uvMinX, pgd.uvMinY);
		fg.uvMax = vec2(pgd.uvMaxX, pgd.uvMaxY);
		
		font.m_glyphs[fg.Codepoint] = fg;
	}

	if (header.compressedSize <= 0 || header.uncompressedSize <= 0)
	{
		Log::Warn("Invalid font atlas sizes -> {0}", filepath.string());
		return;
	}

	std::vector<char> compressed((size_t)header.compressedSize);
	file.read(compressed.data(), header.compressedSize);

	file.close();

	std::vector<unsigned char> decompressed((size_t)header.uncompressedSize);

	const int decompressedSize = LZ4_decompress_safe(
		compressed.data(),
		reinterpret_cast<char*>(decompressed.data()),
		header.compressedSize,
		header.uncompressedSize);

	if (decompressedSize < 0)
	{
		Log::Error("Failed to decompress font atlas: {0}", cachePath);
		return;
	}

	const int channels = 4;
	font.m_tb = std::make_shared<TextureBuffer>(decompressed.data(), font.m_atlasWidth, font.m_atlasHeight, channels);

	Log::Trace("Font uploaded to GPU -> {0} ({1}x{2})", cachePath, font.m_atlasWidth, font.m_atlasHeight);
}

bool Loopie::FontImporter::CheckIfIsFont(const char* path)
{
	std::string ext = ToLower(std::filesystem::path(path).extension().string());
	return ext == ".ttf" || ext == ".otf";
}

void Loopie::FontImporter::WriteRgba(unsigned char* dst, unsigned char a)
{
	dst[0] = 255;
	dst[1] = 255;
	dst[2] = 255;
	dst[3] = a;
}

std::string Loopie::FontImporter::ToLower(std::string s)
{
	std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return (char)std::tolower(c); });
	return s;
}
