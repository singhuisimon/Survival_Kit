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
#include <filesystem>

namespace gam300 {

    // Base path to the assets directory - modify this to match your project structure
    const std::string BASE_ASSETS_PATH = std::filesystem::current_path().string() + "\\Assets\\";

    //Edited - Lily and Huishan (10/9)
    std::string getAssetsPath() {
        std::string formattedBasePath = BASE_ASSETS_PATH;

        for (char& c : formattedBasePath) {
            if (c == '\\') c = '/';
        }

        if (!formattedBasePath.empty() && (formattedBasePath[0] == '/' || formattedBasePath[0] == '\\')) {
            formattedBasePath = formattedBasePath.substr(1);
        }

        return formattedBasePath;
    }

    //Edited - Lily and Huishan (10/9)
    std::string getAssetFilePath(const std::string& relativePath) {
        // Make sure the path uses forward slashes for consistency

        //Debug for the Assets File path
        //std::cout << "Current Path: " << std::filesystem::current_path() << "\n";

        std::string formattedPath = relativePath;
        std::string currentPath = getAssetsPath();

        for (char& c : formattedPath) {
            if (c == '\\') c = '/';
        }

        // Remove leading slash if present to avoid double slash
        if (!formattedPath.empty() && (formattedPath[0] == '/' || formattedPath[0] == '\\')) {
            formattedPath = formattedPath.substr(1);
        }

        return currentPath + formattedPath;
    }

} // end of namespace gam300