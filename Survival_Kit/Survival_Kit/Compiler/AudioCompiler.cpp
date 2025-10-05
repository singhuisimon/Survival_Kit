/**
 * @file AudioCompiler.cpp
 * @brief Implementation of audio compilation.
 * @details Handles audio compression and binary serialization for runtime playback.
 * @author Simon Chan
 * @date October 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#include "AudioCompiler.h"
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

namespace gam300 {

    CompileResult AudioCompiler::compile(
        const std::string& intermediatePath,
        [[maybe_unused]] const ResourceProperties* properties,
        ResourcePaths& paths,
        const xresource::full_guid& guid
    ) {
        CompileResult result;

        LM.writeLog("AudioCompiler - Compiling: %s", intermediatePath.c_str());

        // For now, just copy the intermediate file
        // TODO: Implement audio compression (Vorbis, etc.)

        std::string compiledPath = paths.getCompiledFilePath(guid, ResourceType::AUDIO);
        std::string dir = fs::path(compiledPath).parent_path().string();
        paths.createDirectoryIfNotExists(dir);

        try {
            fs::copy_file(intermediatePath, compiledPath,
                fs::copy_options::overwrite_existing);

            result.success = true;
            result.compiledPath = compiledPath;
            result.originalSize = paths.getFileSize(intermediatePath);
            result.compiledSize = paths.getFileSize(compiledPath);
            result.compressionRatio = 1.0f;
            result.info = "Simple copy (compression not yet implemented)";

            LM.writeLog("AudioCompiler - Success: %s", compiledPath.c_str());
        }
        catch (const std::exception& e) {
            result.error = std::string("Failed to compile audio: ") + e.what();
            LM.writeLog("AudioCompiler - Error: %s", result.error.c_str());
        }

        return result;
    }

} // namespace gam300