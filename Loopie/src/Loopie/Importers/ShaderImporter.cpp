#include "ShaderImporter.h"

#include "Loopie/Core/Log.h"
#include "Loopie/Core/Application.h"

#include <fstream>
#include <iostream>
#include <filesystem>

namespace Loopie {
    void ShaderImporter::ImportShaderAsset(const std::string& filepath, Metadata& metadata)
    {
        if (metadata.HasCache && !metadata.IsOutdated)
            return;

        if (!std::filesystem::exists(filepath))
        {
            Log::Error("Shader not found: {0}", filepath);
            return;
        }

        Project project = Application::GetInstance().m_activeProject;

        UUID id;
        std::filesystem::path extension = std::filesystem::path(filepath).extension();

        std::filesystem::path locationPath = "Shaders";
        locationPath /= id.Get() + extension.string();

        std::filesystem::path destination = project.GetChachePath() / locationPath;

        std::filesystem::create_directories(destination.parent_path());

        bool success = std::filesystem::copy_file(filepath, destination, std::filesystem::copy_options::overwrite_existing);
        if (!success) {
            Log::Error("Failed to copy shader to cache: {0}", filepath);
            return;
        }

        metadata.HasCache = true;
        metadata.CachesPath.clear();
        metadata.CachesPath.push_back(locationPath.string());
        metadata.Type = ResourceType::SHADER;

        MetadataRegistry::SaveMetadata(filepath, metadata);

        Log::Trace("Shader Imported -> {0}", filepath);
    }

    bool ShaderImporter::CheckIfIsShaderAsset(const char* path)
    {
        std::string ext = std::filesystem::path(path).extension().string();

        for (auto& c : ext)
            c = (char)tolower(c);

        return (ext == ".shader");
    }
}