#pragma once

#include "Loopie/Core/UUID.h"
#include "Loopie/Resources/Resource.h"
#include "Loopie/Importers/ImportSettings.h"

#include <filesystem>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <ctime>   
#include <cstdint> // Added for uint64_t

namespace Loopie {

#define METADATA_EXTENSION ".meta"
#define METADATA_CACHE_EXTENSION ".metacache"

    struct Metadata {
        UUID UUID;
        ResourceType Type = ResourceType::UNKNOWN;
        std::vector<std::string> CachesPath;

        bool HasCache = false;
        bool IsOutdated = false;

        uint64_t LastModified = 0;
        std::string LegibleLastModified;

        const std::string& RefreshLegibleLastModified() {
            std::time_t time_sec = static_cast<std::time_t>(LastModified / 1000);
            std::tm* tm_ptr = std::localtime(&time_sec);

            std::ostringstream oss;
            if (tm_ptr) {
                oss << std::put_time(tm_ptr, "%Y-%m-%d %H:%M:%S");
                LegibleLastModified = oss.str();
            }
            else {
                LegibleLastModified = "Invalid Time";
            }

            return LegibleLastModified;
        }
    };

    class MetadataRegistry {
    public:
        static Metadata GetMetadataAsset(const std::string& assetPath);
        static void SaveMetadata(const std::filesystem::path& assetPath, const Metadata& metadata);

        static bool IsMetadataFile(const std::filesystem::path& assetPath);
        static uint64_t GetLastModifiedFromPath(const std::string& assetPath);
    };
}