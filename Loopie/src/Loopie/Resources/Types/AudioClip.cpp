#include "AudioClip.h"

#include "Loopie/Core/Log.h"
#include "Loopie/Resources/AssetRegistry.h"
#include "Loopie/Importers/AudioImporter.h"

#include <fmod_studio.hpp>
#include <fmod.hpp>

namespace Loopie {
	AudioClip::AudioClip(const UUID& id) : Resource(id,ResourceType::AUDIO)
	{
		Load();
	}

	AudioClip::~AudioClip()
	{
		Unload();
	}

	bool AudioClip::Load()
	{
		Metadata* metadata = AssetRegistry::GetMetadata(GetUUID());
		if (!metadata->HasCache)
			return false;

		AudioImporter::LoadAudio(metadata->CachesPath[0], *this);
		return true;
	}

	void AudioClip::Unload()
	{
		if (m_sound)
		{
			m_sound->release();
			m_sound = nullptr;
		}
	}
}