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

namespace fs = std::filesystem;

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

    // For Descriptor Generation for Assets

    //get repo root by finding the Survival_Kit folder structure
    std::string getRepositoryRoot() {
        fs::path currentPath = fs::current_path();

        //search upwards
        while (!currentPath.empty()) {
            //check if able to find Assets folder
            if (fs::exists(currentPath / "Assets")) {
                //verification
                if (currentPath.filename() == "Survival_Kit" ||
                    fs::exists(currentPath / "Survival_Kit.sln") ||
                    fs::exists(currentPath / ".git")) {
                    return currentPath.string();
                }
            }

            //check for nested structure
            if (fs::exists(currentPath / "Survival_Kit" / "Survival_Kit" / "Survival_Kit" / "Assets")) {
                return (currentPath / "Survival_Kit" / "Survival_Kit" / "Survival_Kit").string();
            }

            currentPath = currentPath.parent_path();
        }

        return fs::current_path().string();
     }


    //get local cache directory (for intermediate files, NOT descriptors
    std::string getLocalCachePath() {
        fs::path repoRoot = getRepositoryRoot();
        fs::path cachePath = repoRoot / "Cache";

        // create the cache directory if it dosen't exist
        if (!fs::exists(cachePath)) {
            std::error_code ec;
            fs::create_directories(cachePath, ec);
        }

        return cachePath.generic_string();
    }


    //get intermediate directory for processed assets
    std::string getIntermediatePath()
    {
        fs::path cachePath = getLocalCachePath();
        fs::path intermediatePath = cachePath / "Intermediate";

        //create directory if it dosen't exist
        if (!fs::exists(intermediatePath))
        {
            std::error_code ec;
            fs::create_directories(intermediatePath, ec);
        }

        return intermediatePath.generic_string();
    }


    //build a descriptor file path with the required structure: Assets/Descriptors/AssetType/Dir1/Dir2/GUID.desc/filename
    std::string buildDescriptorPath(const std::string& assetType,
        const std::string& guid,
        const std::string& filename) {
        // Ensure GUID is at least 16 characters
        if (guid.length() < 16) {
            // If GUID is too short, pad with zeros or return error path
            return "";
        }

        // Extract Dir1 (last 2 chars) and Dir2 (second-to-last 2 chars)
        std::string dir1 = guid.substr(14, 2);
        std::string dir2 = guid.substr(12, 2);

        // Get the base Assets path
        fs::path assetsPath = getAssetsPath();

        // Build the complete path: Assets/Descriptors/AssetType/Dir1/Dir2/GUID.desc/
        fs::path descriptorDir = assetsPath / "Descriptors" / assetType / dir1 / dir2 / (guid + ".desc");

        // Create all necessary directories
        if (!fs::exists(descriptorDir)) {
            std::error_code ec;
            fs::create_directories(descriptorDir, ec);
        }

        // Return the full path including the filename
        return (descriptorDir / filename).generic_string();
    }


} // end of namespace gam300