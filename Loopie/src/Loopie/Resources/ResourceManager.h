#pragma once
#include "Loopie/Resources/Resource.h"
#include "Loopie/Resources/Types/Texture.h"
#include "Loopie/Resources/Types/Mesh.h"
#include "Loopie/Resources/Types/Material.h"
#include "Loopie/Resources/Types/AudioClip.h"
#include "Loopie/Resources/Types/Font.h"
#include "Loopie/Resources/Types/SceneAsset.h"
#include "Loopie/Resources/Resource.h"
#include "Loopie/Resources/AssetRegistry.h"

#include <unordered_map>
#include <memory>

namespace Loopie {

    struct ResourceKey {
        Metadata metadata;
        int index = 0;

        bool operator==(const ResourceKey& other) const noexcept {
            return metadata.UUID == other.metadata.UUID && index == other.index;
        }
    };

    struct ResourceKeyHash {
        std::size_t operator()(const ResourceKey& key) const noexcept {
            return std::hash<UUID>()(key.metadata.UUID) ^ (std::hash<int>()(key.index) << 1);
        }
    };

    class ResourceManager {    
    public:
        static std::shared_ptr<Texture> GetTexture(const Metadata& metadata);
        static std::shared_ptr<Mesh> GetMesh(const Metadata& metadata, int index);
        static std::shared_ptr<Material> GetMaterial(const Metadata& metadata);
        static std::shared_ptr<AudioClip> GetAudioClip(const Metadata& metadata);
        static std::shared_ptr<Font> GetFont(const Metadata& metadata);
        static std::shared_ptr<SceneAsset> GetSceneAsset(const Metadata& metadata);

        static void RemoveResource(Resource& resource);

    private:
        static std::shared_ptr<Resource> GetResource(const ResourceKey& key);

    private:
		static std::unordered_map<ResourceKey, std::weak_ptr<Resource>, ResourceKeyHash> s_Resources;
    };
}