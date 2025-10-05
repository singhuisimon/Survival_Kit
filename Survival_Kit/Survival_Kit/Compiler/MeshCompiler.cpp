#include "MeshCompiler.h"
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

namespace gam300 {

    CompileResult MeshCompiler::compile(
        const std::string& intermediatePath,
        [[maybe_unused]] const ResourceProperties* properties,
        ResourcePaths& paths,
        const xresource::full_guid& guid
    ) {
        CompileResult result;

        LM.writeLog("MeshCompiler - Compiling: %s", intermediatePath.c_str());

        // For now, just copy the intermediate file as compiled
        // TODO: Implement actual mesh optimization and binary format

        std::string compiledPath = paths.getCompiledFilePath(guid, ResourceType::MESH);
        std::string dir = fs::path(compiledPath).parent_path().string();
        paths.createDirectoryIfNotExists(dir);

        try {
            // Simple copy for now
            fs::copy_file(intermediatePath, compiledPath,
                fs::copy_options::overwrite_existing);

            result.success = true;
            result.compiledPath = compiledPath;
            result.originalSize = paths.getFileSize(intermediatePath);
            result.compiledSize = paths.getFileSize(compiledPath);
            result.compressionRatio = 1.0f;
            result.info = "Simple copy (optimization not yet implemented)";

            LM.writeLog("MeshCompiler - Success: %s", compiledPath.c_str());
        }
        catch (const std::exception& e) {
            result.error = std::string("Failed to compile mesh: ") + e.what();
            LM.writeLog("MeshCompiler - Error: %s", result.error.c_str());
        }

        return result;
    }

} // namespace gam300