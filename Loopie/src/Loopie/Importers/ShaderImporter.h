#pragma once
#include "Loopie/Resources/Types/ShaderAsset.h"
#include "Loopie/Resources/MetadataRegistry.h"

#include <memory>
#include <vector>
#include <string>

namespace Loopie {

	class ShaderImporter {
	public:
		static void ImportShaderAsset(const std::string& filepath, Metadata& metadata);
		static bool CheckIfIsShaderAsset(const char* path);
	};
}