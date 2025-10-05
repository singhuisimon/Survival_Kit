/**
 * @file SceneImporter.h
 * @brief Declares the scene asset importer class.
 * @author Rio Shannon Yvon Leonardo (100%)
 * @date 22/09/2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */


#pragma once
#include "../AssetImporter.h"

namespace gam300 
{
    class SceneImporter : public IAssetImporter {
    public:

        /**
         * @brief Checks if the importer supports the given scene file extension.
         * @param ext The file extension (e.g., ".scene", ".scn", ".json").
         * @return True if the file type is supported.
         */
        bool CanImport(const std::string& ext) const override;

        /**
         * @brief Imports the scene file and copies it into the intermediate directory.
         * @param srcPath The source file path.
         * @param intermediateDir Directory where the imported file is stored.
         * @return ImportResult containing success status and asset metadata.
         */
        ImportResult Import(const std::string& srcPath,
            const std::string& intermediateDir) override;
    };

} //end of namespace gam300