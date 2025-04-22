/**
 * @file AssetPath.cpp
 * @brief Implementation of asset path management functions.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#include "AssetPath.h"

namespace gam300 {

    // Base path to the assets directory - modify this to match your project structure
    const std::string BASE_ASSETS_PATH = "C:/GitHub/gam_300/gam_300_engine/gam_300_engine/Assets/";

    std::string getAssetsPath() {
        return BASE_ASSETS_PATH;
    }

    std::string getAssetFilePath(const std::string& relativePath) {
        // Make sure the path uses forward slashes for consistency
        std::string formattedPath = relativePath;
        for (char& c : formattedPath) {
            if (c == '\\') c = '/';
        }

        // Remove leading slash if present to avoid double slash
        if (!formattedPath.empty() && (formattedPath[0] == '/' || formattedPath[0] == '\\')) {
            formattedPath = formattedPath.substr(1);
        }

        return BASE_ASSETS_PATH + formattedPath;
    }

} // end of namespace gam300