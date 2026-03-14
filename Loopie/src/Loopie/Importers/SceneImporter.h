#pragma once
#include "Loopie/Resources/Types/SceneAsset.h"
#include "Loopie/Resources/MetadataRegistry.h"

#include <memory>
#include <vector>
#include <string>

namespace Loopie {

	class SceneImporter {
	public:
		static void ImportSceneAsset(const std::string& filepath, Metadata& metadata);
		static bool CheckIfIsSceneAsset(const char* path);
	};
}