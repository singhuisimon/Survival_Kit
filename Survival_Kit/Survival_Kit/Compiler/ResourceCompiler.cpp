/**
 * @file ResourceCompiler.cpp
 * @brief Base interface for resource compilers
 * @details Defines the common interface for all resource type compilers
 * @author Simon Chan
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 */
#include "ResourceCompiler.h"
#include <filesystem>

namespace fs = std::filesystem;

namespace gam300 {

    // ==================== IResourceCompiler ====================

    bool IResourceCompiler::needsRecompile(
        const std::string& intermediatePath,
        const std::string& compiledPath,
        [[maybe_unused]] const ResourceProperties* properties
    ) const {
        // Check if compiled file exists
        if (!fs::exists(compiledPath)) {
            return true; // Need to compile if output doesn't exist
        }

        // Check if intermediate is newer than compiled
        auto intermediateTime = fs::last_write_time(intermediatePath);
        auto compiledTime = fs::last_write_time(compiledPath);

        if (intermediateTime > compiledTime) {
            return true; // Intermediate is newer, recompile
        }

        // All checks passed, no recompile needed
        return false;
    }

    // ==================== CompilerRegistry ====================

    void CompilerRegistry::registerCompiler(std::unique_ptr<IResourceCompiler> compiler) {
        if (compiler) {
            LM.writeLog("CompilerRegistry - Registered compiler for type: %d",
                static_cast<int>(compiler->getResourceType()));
            m_compilers.push_back(std::move(compiler));
        }
    }

    CompileResult CompilerRegistry::compile(
        ResourceType type,
        const std::string& intermediatePath,
        const ResourceProperties* properties,
        ResourcePaths& paths,
        const xresource::full_guid& guid
    ) {
        CompileResult result;

        // Find appropriate compiler
        IResourceCompiler* compiler = getCompiler(type);
        if (!compiler) {
            result.error = "No compiler registered for resource type";
            LM.writeLog("CompilerRegistry - No compiler found for type: %d", static_cast<int>(type));
            return result;
        }

        // Get compiled path
        std::string compiledPath = paths.getCompiledFilePath(guid, type);

        // Check if recompilation is needed
        if (!compiler->needsRecompile(intermediatePath, compiledPath, properties)) {
            result.success = true;
            result.compiledPath = compiledPath;
            result.info = "Up-to-date, skipped compilation";
            LM.writeLog("CompilerRegistry - Skipping compilation (up-to-date): %s",
                intermediatePath.c_str());
            return result;
        }

        // Perform compilation
        LM.writeLog("CompilerRegistry - Compiling: %s", intermediatePath.c_str());

        try {
            result = compiler->compile(intermediatePath, properties, paths, guid);
        }
        catch (const std::exception& e) {
            result.success = false;
            result.error = std::string("Exception during compilation: ") + e.what();
            LM.writeLog("CompilerRegistry - Exception: %s", e.what());
        }

        return result;
    }

    IResourceCompiler* CompilerRegistry::getCompiler(ResourceType type) {
        for (auto& compiler : m_compilers) {
            if (compiler->getResourceType() == type) {
                return compiler.get();
            }
        }
        return nullptr;
    }

} // namespace gam300