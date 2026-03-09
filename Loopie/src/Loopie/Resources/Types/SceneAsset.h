#pragma once
#include "Loopie/Resources/Resource.h"

#include <filesystem>

namespace Loopie {

    class SceneAsset : public Resource {
    public:
        DEFINE_TYPE(SceneAsset)

        SceneAsset(const UUID& id);
        ~SceneAsset() = default;

        bool Load() override;

		const std::filesystem::path& GetSceneFilePath() const { return m_sceneFilePath; }
    private:
		std::filesystem::path m_sceneFilePath;
    };
}