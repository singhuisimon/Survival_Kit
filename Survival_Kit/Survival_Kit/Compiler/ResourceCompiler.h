/**
 * @file ResourceCompiler.h
 * @brief Base interface for resource compilers
 * @details Defines the common interface for all resource type compilers
 * @author Your Name
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 */
#pragma once
#ifndef __RESOURCE_COMPILER_H__
#define __RESOURCE_COMPILER_H__

#include <string>
#include <memory>
#include <vector>
#include "../Resource/ResourceTypes.h"
#include "../Resource/ResourceMetadata.h"
#include "../Resource/ResourcePaths.h"
#include "../Manager/LogManager.h"
#include "../Pipeline/AssetDatabase.h" 

namespace gam300 {

    /**
     * @brief Result of a compilation operation
     */
    struct CompileResult {
        bool success = false;
        std::string compiledPath;      // Path to output .tex/.mesh/etc file
        std::string error;             // Error message if failed
        size_t originalSize = 0;       // Size of intermediate file
        size_t compiledSize = 0;       // Size of compiled file
        float compressionRatio = 1.0f; // compiledSize / originalSize

        std::string info;              // Additional info about compilation
		AssetId assetId = 0;           // Asset ID assigned during compilation 
    };

    /**
     * @brief Common binary format header for all compiled resources
     */
    struct CompiledResourceHeader {
        uint32_t magic;           // Magic number for validation
        uint32_t version;         // Format version
        uint32_t resourceType;    // ResourceType enum value
        uint32_t flags;           // Compression, etc.
        uint64_t dataSize;        // Size of data section
        uint64_t metadataSize;    // Size of metadata section
        uint64_t guid_instance;   // GUID instance value
        uint64_t guid_type;       // GUID type value
        uint32_t checksum;        // CRC32 or similar
        uint32_t reserved;        // For future use

        static constexpr uint32_t MAGIC_NUMBER = 0x52455347; // 'RESG' (Resource Game)
        static constexpr uint32_t CURRENT_VERSION = 1;
    };

    // Flags for compiled resources
    namespace CompileFlags {
        constexpr uint32_t COMPRESSED = 1 << 0;
        constexpr uint32_t HAS_MIPMAPS = 1 << 1;
        constexpr uint32_t SRGB = 1 << 2;
    }

    /**
     * @brief Base compiler interface - all compilers inherit from this
     */
    class IResourceCompiler {
    public:
        virtual ~IResourceCompiler() = default;

        /**
         * @brief Compile a resource from intermediate to final binary format
         * @param intermediatePath Path to intermediate file (from importer)
         * @param properties Resource properties from descriptor
         * @param paths ResourcePaths utility for path resolution
         * @param guid Resource GUID for naming output file
         * @return CompileResult with success/failure info
         */
        virtual CompileResult compile(
            const std::string& intermediatePath,
            const ResourceProperties* properties,
            ResourcePaths& paths,
            const xresource::full_guid& guid
        ) = 0;

        /**
         * @brief Get the resource type this compiler handles
         */
        virtual ResourceType getResourceType() const = 0;

        /**
         * @brief Check if recompilation is needed
         */
        virtual bool needsRecompile(
            const std::string& intermediatePath,
            const std::string& compiledPath,
            const ResourceProperties* properties
        ) const;
    };

    /**
     * @brief Registry for managing multiple compilers
     */
    class CompilerRegistry {
    public:
        void registerCompiler(std::unique_ptr<IResourceCompiler> compiler);

        CompileResult compile(
            ResourceType type,
            const std::string& intermediatePath,
            const ResourceProperties* properties,
            ResourcePaths& paths,
            const xresource::full_guid& guid
        );

        IResourceCompiler* getCompiler(ResourceType type);

    private:
        std::vector<std::unique_ptr<IResourceCompiler>> m_compilers;
    };

} // namespace gam300

#endif // __RESOURCE_COMPILER_H__