/**
 * @file MeshImporter.cpp
 * @brief Implements mesh importing functionality.
 * @author Rio Shannon Yvon Leonardo (100%)
 * @date 22/09/2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */


#include "MeshImporter.h"
#include <filesystem>

namespace fs = std::filesystem;

namespace gam300 
{

    bool MeshImporter::CanImport(const std::string& ext) const {
        std::string lower = ext;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

        return (lower == ".obj" || lower == ".fbx" || lower == ".gltf");
    }

    ImportResult MeshImporter::Import(const std::string& srcPath,
        const std::string& intermediateDir) {
        ImportResult r;
        try {
            fs::path src(srcPath);
            fs::path out = fs::path(intermediateDir) / src.filename();

            fs::create_directories(out.parent_path());
            fs::copy_file(src, out, fs::copy_options::overwrite_existing);

            r.ok = true;
            r.intermediatePath = out.string();
            r.type = AssetType::Mesh;
        }
        catch (const std::exception& e) {
            r.ok = false;
            r.error = e.what();
        }
        return r;
    }

} // end of namespace gam300