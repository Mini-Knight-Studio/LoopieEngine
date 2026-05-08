#include "ResourceManager.h"



namespace Loopie {
	std::unordered_map<ResourceKey, std::weak_ptr<Resource>, ResourceKeyHash> ResourceManager::s_Resources;
    std::unordered_map<InstancedResourceKey, std::weak_ptr<Resource>, InstancedResourceKeyHash> ResourceManager::s_InstancedResources;
	std::vector<std::shared_ptr<Resource>> ResourceManager::s_ProtectedResources;

    std::shared_ptr<Texture> ResourceManager::GetTexture(const Metadata& metadata) {
        ResourceKey key{ metadata, 0 };
        auto resource = GetResource(key);
        if (resource) {
            return std::static_pointer_cast<Texture>(resource);
        }
        auto texture = std::make_shared<Texture>(metadata.UUID);
        s_Resources[key] = texture;
        return texture;
    }

    std::shared_ptr<Mesh> ResourceManager::GetMesh(const Metadata& metadata, int index) {
        ResourceKey key{ metadata, index };
        auto resource = GetResource(key);
        if (resource) {
            return std::static_pointer_cast<Mesh>(resource);
        }
        auto mesh = std::make_shared<Mesh>(metadata.UUID, index);
        s_Resources[key] = mesh;
        return mesh;
    }

    std::shared_ptr<Material> ResourceManager::GetMaterial(const Metadata& metadata)
    {
        ResourceKey key{ metadata, 0 };
        auto resource = GetResource(key);
        if (resource) {
            return std::static_pointer_cast<Material>(resource);
        }
        auto material = std::make_shared<Material>(metadata.UUID);
        s_Resources[key] = material;
        return material;
    }

    std::shared_ptr<AudioClip> ResourceManager::GetAudioClip(const Metadata& metadata)
    {
        ResourceKey key{ metadata, 0 };
        auto resource = GetResource(key);
        if (resource) {
            return std::static_pointer_cast<AudioClip>(resource);
        }
        auto audioClip = std::make_shared<AudioClip>(metadata.UUID);
        s_Resources[key] = audioClip;
        return audioClip;
    }

    std::shared_ptr<Font> ResourceManager::GetFont(const Metadata& metadata)
    {
        ResourceKey key{ metadata, 0 };
        auto resource = GetResource(key);
        if (resource) {
            return std::static_pointer_cast<Font>(resource);
        }
        auto font = std::make_shared<Font>(metadata.UUID);
        s_Resources[key] = font;
        return font;
    }

    std::shared_ptr<SceneAsset> ResourceManager::GetSceneAsset(const Metadata& metadata)
    {
        ResourceKey key{ metadata, 0 };
        auto resource = GetResource(key);
        if (resource) {
            return std::static_pointer_cast<SceneAsset>(resource);
        }
        auto sceneAsset = std::make_shared<SceneAsset>(metadata.UUID);
        s_Resources[key] = sceneAsset;
        return sceneAsset;
    }

    std::shared_ptr<ShaderAsset> ResourceManager::GetShaderAsset(const Metadata& metadata)
    {
        ResourceKey key{ metadata, 0 };
        auto resource = GetResource(key);
        if (resource) {
            return std::static_pointer_cast<ShaderAsset>(resource);
        }
        auto shaderAsset = std::make_shared<ShaderAsset>(metadata.UUID);
        s_Resources[key] = shaderAsset;
        return shaderAsset;
    }

    std::shared_ptr<Resource> ResourceManager::GetResource(const ResourceKey& key)
    {
        auto it = s_Resources.find(key);
        if (it != s_Resources.end())
        {
            if (auto shared = it->second.lock())
                return shared;

            s_Resources.erase(it);
        }
        return nullptr;
    }

    void ResourceManager::RemoveResource(Resource& resource)
    {
        for (auto it = s_Resources.begin(); it != s_Resources.end(); )
        {
            if (auto shared = it->second.lock())
            {
                if (shared.get() == &resource)
                    it = s_Resources.erase(it);
                else
                    ++it;
            }
            else
                it = s_Resources.erase(it);
        }
    }

    std::vector<std::shared_ptr<Resource>> ResourceManager::GetResourcesByType(ResourceKey key)
    {
        std::vector<std::shared_ptr<Resource>> resources;
        for (const auto& pair : s_Resources)
        {
            if (pair.first.metadata.Type == key.metadata.Type)
            {
                if (auto shared = pair.second.lock())
                    resources.push_back(shared);
            }
        }
        return resources;
    }

    std::shared_ptr<Material> ResourceManager::GetInstancedMaterial(const UUID& uuid)
    {
        InstancedResourceKey key{ uuid, 0 };
        auto resource = GetInstancedResource(key);
        if (resource) {
            return std::static_pointer_cast<Material>(resource);
        }
        return nullptr;
    }

    void ResourceManager::AddInstancedResource(const std::shared_ptr<Resource>& resource, int index)
    {
        if (!resource)
            return;
        InstancedResourceKey key{ resource->GetUUID(), index };
        s_InstancedResources[key] = resource;

    }

    void ResourceManager::RemoveInstancedResource(Resource& resource)
    {
        for (auto it = s_InstancedResources.begin(); it != s_InstancedResources.end(); )
        {
            if (auto shared = it->second.lock())
            {
                if (shared.get() == &resource)
                    it = s_InstancedResources.erase(it);
                else
                    ++it;
            }
            else
                it = s_InstancedResources.erase(it);
        }
    }

    std::shared_ptr<Resource> ResourceManager::GetInstancedResource(const InstancedResourceKey& key)
    {
        auto it = s_InstancedResources.find(key);
        if (it != s_InstancedResources.end())
        {
            if (auto shared = it->second.lock())
                return shared;

            s_InstancedResources.erase(it);
        }
        return nullptr;
    }


    void ResourceManager::ProtectResources()
    {
        s_ProtectedResources.clear();
        s_ProtectedResources.reserve(s_Resources.size());

        for (auto it = s_Resources.begin(); it != s_Resources.end(); )
        {
            if (auto shared = it->second.lock())
            {
                s_ProtectedResources.push_back(shared);
                ++it;
            }
            else
            {
                it = s_Resources.erase(it);
            }
        }
    }
    void ResourceManager::UnprotectResources()
    {
        s_ProtectedResources.clear();
    }
}
