#include "MetadataRegistry.h"

#include "Loopie/Core/Log.h"
#include "Loopie/Core/Application.h"
#include "Loopie/Files/Json.h"
#include "Loopie/Files/DirectoryManager.h"

namespace Loopie {
	Metadata MetadataRegistry::GetMetadataAsset(const std::string& assetPath)
	{
		std::filesystem::path metadataPath = assetPath + METADATA_EXTENSION;
		std::filesystem::path metadataCachePath = assetPath + METADATA_CACHE_EXTENSION;

        Metadata metadata;
        bool metadataExists = std::filesystem::exists(metadataPath);
        bool metadataCacheExists = std::filesystem::exists(metadataCachePath);
        if (!metadataExists || !metadataCacheExists) {
            if (metadataExists) {
                JsonData metadataData = Json::ReadFromFile(metadataPath);
                metadata.UUID = UUID(metadataData.GetValue<std::string>("Id").Result);
                metadata.Type = (ResourceType)metadataData.GetValue<int>("Type").Result;
            }
            metadata.IsOutdated = true;
            metadata.LastModified = GetLastModifiedFromPath(assetPath);
            SaveMetadata(assetPath, metadata);
            return metadata;
        }
        else {
            JsonData metadataData = Json::ReadFromFile(metadataPath);
            JsonData metadataCacheData = Json::ReadFromFile(metadataCachePath);

            metadata.UUID = UUID(metadataData.GetValue<std::string>("Id").Result);
            metadata.Type = (ResourceType)metadataData.GetValue<int>("Type").Result;
            metadata.HasCache = metadataCacheData.GetValue<bool>("HasCache").Result;
            metadata.LastModified = metadataCacheData.GetValue<uint64_t>("LastModified").Result;
            uint64_t currentTime = GetLastModifiedFromPath(assetPath);
            metadata.IsOutdated = currentTime != metadata.LastModified;

            if (metadata.HasCache) {
                JsonNode cacheNode = metadataCacheData.Child("Caches");
                unsigned int entries = cacheNode.Size();
                Project project = Application::GetInstance().m_activeProject;
                for (unsigned int i = 0; i < entries; i++)
                {
                    std::string cachePath = cacheNode.GetArrayElement<std::string>(i).Result;
                    if (!std::filesystem::exists(project.GetChachePath() / cachePath))
                    {
                        metadata.CachesPath.clear();
                        metadata.HasCache = false;
                        break;
                    }
                    metadata.CachesPath.push_back(cachePath);
                }
            }
            metadata.RefreshLegibleLastModified();
            return metadata;
        }
	}

	void MetadataRegistry::SaveMetadata(const std::filesystem::path& assetPath, const Metadata& metadata)
	{
        std::filesystem::path metadataPath = assetPath.string() + METADATA_EXTENSION;
        std::filesystem::path metadataCachePath = assetPath.string() + METADATA_CACHE_EXTENSION;

        JsonData metadataData;
        JsonData metadataCacheData;

        metadataData.CreateField<std::string>("Id",metadata.UUID.Get());
        metadataData.CreateField<int>("Type",metadata.Type);
        metadataCacheData.CreateField<bool>("HasCache", metadata.HasCache);
        metadataCacheData.CreateField<uint64_t>("LastModified", metadata.LastModified);

        if (metadata.HasCache) {
            metadataCacheData.CreateArrayField("Caches");
            for (const auto& paths : metadata.CachesPath)
            {
                metadataCacheData.AddArrayElement<std::string>("Caches", paths);
            }
        }
       
        metadataData.ToFile(metadataPath);
        metadataCacheData.ToFile(metadataCachePath);
	}
    bool MetadataRegistry::IsMetadataFile(const std::filesystem::path& assetPath)
    {
        auto extension = assetPath.extension();
        return extension == METADATA_EXTENSION || extension == METADATA_CACHE_EXTENSION;
    }

    uint64_t MetadataRegistry::GetLastModifiedFromPath(const std::string& assetPath) {
        if (!std::filesystem::exists(assetPath)) {
            return 0;
        }

        auto ftime = std::filesystem::last_write_time(assetPath);
        auto duration = ftime.time_since_epoch();
        return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
    }
}