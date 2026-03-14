#include "SceneAsset.h"

#include "Loopie/Core/Log.h"
#include "Loopie/Resources/AssetRegistry.h"

#include "Loopie/Files/Json.h"

namespace Loopie {

    SceneAsset::SceneAsset(const UUID& id) : Resource(id, ResourceType::SCENE)
    {
        Load();
	}

    bool SceneAsset::Load()
    {
        Metadata* metadata = AssetRegistry::GetMetadata(GetUUID());

        if (!metadata->HasCache)
            return false;

        m_sceneFilePath = metadata->CachesPath[0];

        return !m_sceneFilePath.empty();
    }
}