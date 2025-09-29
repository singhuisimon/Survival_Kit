#pragma once
#include "../AssetImporter.h"

namespace gam300 
{
    class SceneImporter : public IAssetImporter {
    public:
        bool CanImport(const std::string& ext) const override;
        ImportResult Import(const std::string& srcPath,
            const std::string& intermediateDir) override;
    };

} //end of namespace gam300