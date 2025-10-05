/**
 * @file TextureCompiler.h
 * @brief Texture resource compiler.
 * @details Compiles intermediate textures to optimized binary format with mipmap generation.
 * @author Simon Chan
 * @date October 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once
#ifndef __TEXTURE_COMPILER_H__
#define __TEXTURE_COMPILER_H__

#include "ResourceCompiler.h"

namespace gam300 {

    /**
     * @brief Compiled texture data format
     */
    struct CompiledTextureData {
        uint32_t width;
        uint32_t height;
        uint32_t channels;      // 3=RGB, 4=RGBA
        uint32_t format;        // Internal format (0=uncompressed for now)
        uint32_t mipLevels;     // Number of mip levels
        bool srgb;              // Is sRGB color space
        uint32_t reserved[2];   // Padding for future use

        // Followed by:
        // - Mip level 0 data
        // - Mip level 1 data
        // - ...
    };

    /**
     * @brief Texture compiler implementation
     */
    class TextureCompiler : public IResourceCompiler {
    public:
        TextureCompiler() = default;
        ~TextureCompiler() override = default;

        CompileResult compile(
            const std::string& intermediatePath,
            const ResourceProperties* properties,
            ResourcePaths& paths,
            const xresource::full_guid& guid
        ) override;

        ResourceType getResourceType() const override {
            return ResourceType::TEXTURE;
        }

    private:
        /**
         * @brief Load image from intermediate file using stb_image
         */
        bool loadImage(const std::string& path, int& width, int& height,
            int& channels, std::vector<unsigned char>& data);

        /**
         * @brief Generate mipmaps for texture
         */
        std::vector<std::vector<unsigned char>> generateMipmaps(
            const std::vector<unsigned char>& data,
            int width, int height, int channels, int levels
        );

        /**
         * @brief Calculate number of mip levels
         */
        int calculateMipLevels(int width, int height) const;

        /**
         * @brief Write compiled texture to binary file
         */
        bool writeCompiledTexture(
            const std::string& outputPath,
            const CompiledTextureData& header,
            const std::vector<std::vector<unsigned char>>& mipData,
            const xresource::full_guid& guid
        );

        /**
         * @brief Calculate simple CRC32 checksum
         */
        uint32_t calculateCRC32(const unsigned char* data, size_t length);
    };

} // namespace gam300

#endif // __TEXTURE_COMPILER_H__