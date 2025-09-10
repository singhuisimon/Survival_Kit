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
#include <iostream>
#include <filesystem>

namespace gam300 {

    // Base path to the assets directory - modify this to match your project structure
    const std::string BASE_ASSETS_PATH = "..\\..\\Survival_Kit\\Assets\\";

    std::string getAssetsPath() {
        return BASE_ASSETS_PATH;
    }

    std::string getAssetFilePath(const std::string& relativePath) {
        // Make sure the path uses forward slashes for consistency

        //Debug for the Assets File path
        //std::cout << "Current Path: " << std::filesystem::current_path() << "\n";

        std::string formattedPath = relativePath;
        std::string currentPath = std::filesystem::current_path().string() + "\\Assets\\";

        for (char& c : formattedPath) {
            if (c == '\\') c = '/';
        }

        // Remove leading slash if present to avoid double slash
        if (!formattedPath.empty() && (formattedPath[0] == '/' || formattedPath[0] == '\\')) {
            formattedPath = formattedPath.substr(1);
        }

        for (char& c : currentPath) {
            if (c == '\\') c = '/';
        }

        // Remove leading slash if present to avoid double slash
        if (!currentPath.empty() && (currentPath[0] == '/' || currentPath[0] == '\\')) {
            currentPath = currentPath.substr(1);
        }

        return currentPath + formattedPath;
    }

} // end of namespace gam300