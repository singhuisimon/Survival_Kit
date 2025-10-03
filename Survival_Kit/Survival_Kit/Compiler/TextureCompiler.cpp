/**
 * @file TextureCompiler.cpp
 * @brief Texture compiler implementation
 */

#include "TextureCompiler.h"
#include <fstream>
#include <cmath>
#include <algorithm>
#include <filesystem>

 // Include stb_image (make sure it's in your include folder)
#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"

namespace fs = std::filesystem;

namespace gam300 {

    CompileResult TextureCompiler::compile(
        const std::string& intermediatePath,
        const ResourceProperties* properties,
        ResourcePaths& paths,
        const xresource::full_guid& guid
    ) {
        CompileResult result;

        // Check if intermediate file exists
        if (!fs::exists(intermediatePath)) {
            result.error = "Intermediate file not found: " + intermediatePath;
            LM.writeLog("TextureCompiler - %s", result.error.c_str());
            return result;
        }

        result.originalSize = paths.getFileSize(intermediatePath);

        // Cast to texture properties
        const TextureProperties* texProps = cast_properties<TextureProperties>(properties);
        if (!texProps) {
            result.error = "Invalid texture properties";
            LM.writeLog("TextureCompiler - Invalid properties for compilation");
            return result;
        }

        LM.writeLog("TextureCompiler - Compiling: %s", intermediatePath.c_str());

        // Load the image
        int width, height, channels;
        std::vector<unsigned char> imageData;

        if (!loadImage(intermediatePath, width, height, channels, imageData)) {
            result.error = "Failed to load image: " + intermediatePath;
            LM.writeLog("TextureCompiler - %s", result.error.c_str());
            return result;
        }

        LM.writeLog("TextureCompiler - Loaded image: %dx%d, %d channels",
            width, height, channels);

        // Generate mipmaps if requested
        std::vector<std::vector<unsigned char>> mipChain;
        int mipLevels = 1;

        if (texProps->generateMipmaps) {
            mipLevels = calculateMipLevels(width, height);
            mipChain = generateMipmaps(imageData, width, height, channels, mipLevels);
            LM.writeLog("TextureCompiler - Generated %d mip levels", mipLevels);
        }
        else {
            mipChain.push_back(imageData);
        }

        // Prepare compiled texture data
        CompiledTextureData texData{};
        texData.width = width;
        texData.height = height;
        texData.channels = channels;
        texData.format = 0; // 0 = uncompressed
        texData.mipLevels = mipLevels;
        texData.srgb = texProps->srgb;

        // Get output path
        std::string compiledPath = paths.getCompiledFilePath(guid, ResourceType::TEXTURE);

        // Ensure directory exists
        std::string dir = fs::path(compiledPath).parent_path().string();
        if (!paths.createDirectoryIfNotExists(dir)) {
            result.error = "Failed to create output directory: " + dir;
            LM.writeLog("TextureCompiler - %s", result.error.c_str());
            return result;
        }

        // Write compiled file
        if (!writeCompiledTexture(compiledPath, texData, mipChain, guid)) {
            result.error = "Failed to write compiled texture";
            LM.writeLog("TextureCompiler - %s", result.error.c_str());
            return result;
        }

        // Success!
        result.success = true;
        result.compiledPath = compiledPath;
        result.compiledSize = paths.getFileSize(compiledPath);
        result.compressionRatio = static_cast<float>(result.compiledSize) /
            static_cast<float>(result.originalSize);

        LM.writeLog("TextureCompiler - Success! Output: %s (%.2f compression ratio)",
            compiledPath.c_str(), result.compressionRatio);

        return result;
    }

    bool TextureCompiler::loadImage(const std::string& path, int& width, int& height,
        int& channels, std::vector<unsigned char>& data) {
        // Use stb_image to load
        unsigned char* imgData = stbi_load(path.c_str(), &width, &height, &channels, 0);

        if (!imgData) {
            LM.writeLog("TextureCompiler - stbi_load failed: %s", stbi_failure_reason());
            return false;
        }

        // Copy to vector
        size_t dataSize = static_cast<size_t>(width) * height * channels;
        data.assign(imgData, imgData + dataSize);

        // Free stb_image data
        stbi_image_free(imgData);

        return true;
    }

    std::vector<std::vector<unsigned char>> TextureCompiler::generateMipmaps(
        const std::vector<unsigned char>& data,
        int width, int height, int channels, int levels
    ) {
        std::vector<std::vector<unsigned char>> mips;
        mips.reserve(levels);

        // Level 0 is the original
        mips.push_back(data);

        int currentWidth = width;
        int currentHeight = height;

        // Generate each subsequent level
        for (int level = 1; level < levels; ++level) {
            int newWidth = std::max(1, currentWidth / 2);
            int newHeight = std::max(1, currentHeight / 2);

            std::vector<unsigned char> mipData(static_cast<size_t>(newWidth) * newHeight * channels);

            // Simple box filter downsampling
            for (int y = 0; y < newHeight; ++y) {
                for (int x = 0; x < newWidth; ++x) {
                    // Sample 2x2 block from previous level
                    int srcX = x * 2;
                    int srcY = y * 2;

                    for (int c = 0; c < channels; ++c) {
                        int sum = 0;
                        int count = 0;

                        // Sample up to 2x2 pixels
                        for (int dy = 0; dy < 2 && (srcY + dy) < currentHeight; ++dy) {
                            for (int dx = 0; dx < 2 && (srcX + dx) < currentWidth; ++dx) {
                                size_t srcIdx = static_cast<size_t>((srcY + dy) * currentWidth + (srcX + dx)) * channels + c;
                                sum += mips[level - 1][srcIdx];
                                count++;
                            }
                        }

                        size_t dstIdx = static_cast<size_t>(y * newWidth + x) * channels + c;
                        mipData[dstIdx] = static_cast<unsigned char>(sum / count);
                    }
                }
            }

            mips.push_back(std::move(mipData));
            currentWidth = newWidth;
            currentHeight = newHeight;
        }

        return mips;
    }

    int TextureCompiler::calculateMipLevels(int width, int height) const {
        int maxDim = std::max(width, height);
        return static_cast<int>(std::floor(std::log2(maxDim))) + 1;
    }

    bool TextureCompiler::writeCompiledTexture(
        const std::string& outputPath,
        const CompiledTextureData& texData,
        const std::vector<std::vector<unsigned char>>& mipData,
        const xresource::full_guid& guid
    ) {
        std::ofstream file(outputPath, std::ios::binary);
        if (!file.is_open()) {
            LM.writeLog("TextureCompiler - Failed to open output file: %s", outputPath.c_str());
            return false;
        }

        // Write header
        CompiledResourceHeader header{};
        header.magic = CompiledResourceHeader::MAGIC_NUMBER;
        header.version = CompiledResourceHeader::CURRENT_VERSION;
        header.resourceType = static_cast<uint32_t>(ResourceType::TEXTURE);
        header.guid_instance = guid.m_Instance.m_Value;
        header.guid_type = guid.m_Type.m_Value;

        // Calculate data size
        size_t totalDataSize = 0;
        for (const auto& mip : mipData) {
            totalDataSize += mip.size();
        }
        header.dataSize = totalDataSize;
        header.metadataSize = sizeof(CompiledTextureData);

        // Set flags
        header.flags = 0;
        if (texData.mipLevels > 1) {
            header.flags |= CompileFlags::HAS_MIPMAPS;
        }
        if (texData.srgb) {
            header.flags |= CompileFlags::SRGB;
        }

        // Calculate checksum (simple CRC32 of all mip data)
        std::vector<unsigned char> allData;
        for (const auto& mip : mipData) {
            allData.insert(allData.end(), mip.begin(), mip.end());
        }
        header.checksum = calculateCRC32(allData.data(), allData.size());
        header.reserved = 0;

        // Write header
        file.write(reinterpret_cast<const char*>(&header), sizeof(header));

        // Write metadata
        file.write(reinterpret_cast<const char*>(&texData), sizeof(texData));

        // Write mip data
        for (const auto& mip : mipData) {
            file.write(reinterpret_cast<const char*>(mip.data()), mip.size());
        }

        file.close();
        return true;
    }

    uint32_t TextureCompiler::calculateCRC32(const unsigned char* data, size_t length) {
        // Simple CRC32 implementation
        uint32_t crc = 0xFFFFFFFF;

        for (size_t i = 0; i < length; ++i) {
            crc ^= data[i];
            for (int j = 0; j < 8; ++j) {
                crc = (crc >> 1) ^ (0xEDB88320 & -(crc & 1));
            }
        }

        return ~crc;
    }

} // namespace gam300