/**
 * @file AudioImporter.cpp
 * @brief Implements audio file importing logic.
 * @author Rio Shannon Yvon Leonardo (100%)
 * @date 22/09/2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */


#include "AudioImporter.h"
#include <filesystem>

namespace fs = std::filesystem;

namespace gam300 
{

    bool AudioImporter::CanImport(const std::string& ext) const {
        std::string lower = ext;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        return (lower == ".wav" || lower == ".ogg" || lower == ".mp3");
    }

    ImportResult AudioImporter::Import(const std::string& srcPath,
        const std::string& intermediateDir) {
        ImportResult r;
        try {
            fs::path src(srcPath);
            fs::path out = fs::path(intermediateDir) / src.filename();

            fs::create_directories(out.parent_path());
            fs::copy_file(src, out, fs::copy_options::overwrite_existing);

            r.ok = true;
            r.intermediatePath = out.string();
            r.type = AssetType::Audio;
        }
        catch (const std::exception& e) {
            r.ok = false;
            r.error = e.what();
        }
        return r;
    }

} // end of namespace gam300