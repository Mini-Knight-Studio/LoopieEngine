#include "Font.h"

#include "Loopie/Resources/AssetRegistry.h"
#include "Loopie/Importers/FontImporter.h"

Loopie::Font::Font(const UUID& id) : Resource(id, ResourceType::FONT)
{
	Load();
}

bool Loopie::Font::Load()
{
	Metadata* metadata = AssetRegistry::GetMetadata(GetUUID());
	if (!metadata || !metadata->HasCache || metadata->CachesPath.empty()) {
		return false;
	}
	FontImporter::LoadFont(metadata->CachesPath[0], *this);
	return true;
}

const Loopie::FontGlyph* Loopie::Font::GetGlyph(int codepoint) const
{
	auto it = m_glyphs.find(codepoint);
	if (it == m_glyphs.end())
		return nullptr;

	return &it->second;
}
