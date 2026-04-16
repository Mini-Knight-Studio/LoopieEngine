#include "ShaderAsset.h"

#include "Loopie/Core/Log.h"
#include "Loopie/Resources/AssetRegistry.h"

#include "Loopie/Files/Json.h"

namespace Loopie {

    ShaderAsset::ShaderAsset(const UUID& id) : Resource(id, ResourceType::SHADER)
    {
        Load();
    }

    bool ShaderAsset::Load()
    {
        Metadata* metadata = AssetRegistry::GetMetadata(GetUUID());

        if (!metadata->HasCache)
            return false;

        m_shaderFilePath = metadata->CachesPath[0];

        return !m_shaderFilePath.empty();
    }
}