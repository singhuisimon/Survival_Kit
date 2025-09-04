/**
 * @file AssetPath.h
 * @brief Helper functions for asset path management.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once
#ifndef __ASSET_PATH_H__
#define __ASSET_PATH_H__

#include <string>

namespace gam300 {

    /**
     * @brief Get the absolute path to the assets directory.
     * @return The absolute path to the assets directory.
     */
    std::string getAssetsPath();

    /**
     * @brief Get the absolute path to a file in the assets directory.
     * @param relativePath The path relative to the assets directory.
     * @return The absolute path to the asset file.
     */
    std::string getAssetFilePath(const std::string& relativePath);

} // end of namespace gam300
#endif // __ASSET_PATH_H__