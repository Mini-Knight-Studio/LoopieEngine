#pragma once
#include "Loopie/Resources/Resource.h"

#include <filesystem>

namespace Loopie {

    class ShaderAsset : public Resource {
    public:
        DEFINE_TYPE(ShaderAsset)

        ShaderAsset(const UUID& id);
        ~ShaderAsset() = default;

        bool Load() override;

        const std::filesystem::path& GetShaderFilePath() const { return m_shaderFilePath; }
    private:
        std::filesystem::path m_shaderFilePath;
    };
}