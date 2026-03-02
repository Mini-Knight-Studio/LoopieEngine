#include "AudioImporter.h"

#include "Loopie/Core/Log.h"
#include "Loopie/Core/Application.h"
#include "Loopie/Core/AudioManager.h"

#include <fmod_studio.hpp>
#include <fmod.hpp>

#include <fstream>
#include <iostream>
#include <filesystem>

namespace Loopie {
	void AudioImporter::ImportAudio(const std::string& filepath, Metadata& metadata) {
        if (metadata.HasCache && !metadata.IsOutdated)
            return;

        if (!std::filesystem::exists(filepath))
        {
            Log::Error("Audio file not found: {0}", filepath);
            return;
        }

        Project project = Application::GetInstance().m_activeProject;

        UUID id;
        std::filesystem::path extension = std::filesystem::path(filepath).extension();

        std::filesystem::path locationPath = "Audio";
        locationPath /= id.Get() + extension.string();

        std::filesystem::path destination = project.GetChachePath() / locationPath;

        std::filesystem::create_directories(destination.parent_path());

        try
        {
            std::filesystem::copy_file(filepath, destination, std::filesystem::copy_options::overwrite_existing);
        }
        catch (...)
        {
            Log::Error("Failed to copy audio to cache: {0}", filepath);
            return;
        }

        metadata.HasCache = true;
        metadata.CachesPath.clear();
        metadata.CachesPath.push_back(locationPath.string());
        metadata.Type = ResourceType::AUDIO;

        MetadataRegistry::SaveMetadata(filepath, metadata);

        Log::Trace("Audio Imported -> {0}", filepath);
	}

	void AudioImporter::LoadAudio(const std::string& filepath, AudioClip& audioClip) {

        Project project = Application::GetInstance().m_activeProject;
        std::filesystem::path fullPath = project.GetChachePath() / filepath;

        if (!std::filesystem::exists(fullPath))
        {
            Log::Warn("Audio cache file not found: {0}", fullPath.string());
            return;
        }

        uintmax_t size = std::filesystem::file_size(fullPath);
        bool stream = size > (1024 * 1024 * 2);

        audioClip.m_sound = AudioManager::CreateSound(fullPath.string(),false, stream);

        if (!audioClip.m_sound)
            Log::Warn("Failed to create FMOD sound: {0}", fullPath.string());
	}

	bool AudioImporter::CheckIfIsAudio(const char* path) {

        std::string ext = std::filesystem::path(path).extension().string();

        for (auto& c : ext)
            c = (char)tolower(c);

        return (ext == ".wav" || ext == ".mp3" || ext == ".ogg" || ext == ".flac");
	}
}