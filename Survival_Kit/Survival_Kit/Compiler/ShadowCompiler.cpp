#include "ShaderCompiler.h"
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

namespace gam300 {

    CompileResult ShaderCompiler::compile(
        const std::string& intermediatePath,
        [[maybe_unused]] const ResourceProperties* properties,
        ResourcePaths& paths,
        const xresource::full_guid& guid
    ) {
        CompileResult result;

        LM.writeLog("ShaderCompiler - Compiling: %s", intermediatePath.c_str());

        // For now, just copy the shader source
        // TODO: Implement shader validation and optimization

        std::string compiledPath = paths.getCompiledFilePath(guid, ResourceType::SHADER);
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
            result.info = "Simple copy (validation not yet implemented)";

            LM.writeLog("ShaderCompiler - Success: %s", compiledPath.c_str());
        }
        catch (const std::exception& e) {
            result.error = std::string("Failed to compile shader: ") + e.what();
            LM.writeLog("ShaderCompiler - Error: %s", result.error.c_str());
        }

        return result;
    }

} // namespace gam300