/**
 * @file ResourceLoaders.cpp
 * @brief Complete implementation of xresource_mgr loaders with OpenGL integration
 * @details Loads compiled binary resources and creates OpenGL handles
 * @author Wai Lwin Thit
 * @date October 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 */

#include "ResourceData.h"
#include "../Manager/ResourceManager.h"
#include "../Manager/LogManager.h"
#include "../compiler/ResourceCompiler.h"      // Base: CompiledResourceHeader
#include "../compiler/TextureCompiler.h"       // CompiledTextureData
#include "../compiler/MeshCompiler.h"          // CompiledMeshData (if needed)
#include "../compiler/AudioCompiler.h"         // CompiledAudioData (if needed)
#include "../compiler/ShaderCompiler.h"        // CompiledShaderData (if needed)
#include "../Glad/glad.h" // OpenGL functions
#include <fstream>
#include <memory>


 // Loader registrations - defined once
xresource::loader_registration<gam300::ResourceGUID::texture_type_guid_v> texture_loader;
xresource::loader_registration<gam300::ResourceGUID::mesh_type_guid_v> mesh_loader;
xresource::loader_registration<gam300::ResourceGUID::material_type_guid_v> material_loader;
xresource::loader_registration<gam300::ResourceGUID::audio_type_guid_v> audio_loader;
xresource::loader_registration<gam300::ResourceGUID::shader_type_guid_v> shader_loader;

namespace gam300 {

    // Helper to get ResourceManager from xresource::mgr
    ResourceManager* getResourceManager(xresource::mgr& mgr) {
        return &mgr.getUserData<ResourceManager>();
    }

    // Helper to read compiled resource header
    bool readCompiledHeader(std::ifstream& file, CompiledResourceHeader& header) {
        file.read(reinterpret_cast<char*>(&header), sizeof(CompiledResourceHeader));

        if (!file) {
            LM.writeLog("ResourceLoader - Failed to read compiled resource header");
            return false;
        }

        // Validate magic number
        if (header.magic != CompiledResourceHeader::MAGIC_NUMBER) {
            LM.writeLog("ResourceLoader - Invalid magic number: 0x%X", header.magic);
            return false;
        }

        // Check version compatibility
        if (header.version != CompiledResourceHeader::CURRENT_VERSION) {
            LM.writeLog("ResourceLoader - Unsupported version: %u", header.version);
            return false;
        }

        return true;
    }

} // namespace gam300


// ========== TEXTURE LOADER IMPLEMENTATION ==========

xresource::loader<gam300::ResourceGUID::texture_type_guid_v>::data_type*
xresource::loader<gam300::ResourceGUID::texture_type_guid_v>::Load(
    xresource::mgr& mgr, const full_guid& guid)
{
    gam300::ResourceManager* rm = gam300::getResourceManager(mgr);

    // Get compiled file path
    std::string compiled_path = rm->getPaths().getCompiledFilePath(guid, gam300::ResourceType::TEXTURE);

    if (!rm->getPaths().fileExists(compiled_path)) {
        LM.writeLog("TextureLoader - Compiled file not found: %s", compiled_path.c_str());
        return nullptr;
    }

    // Open compiled binary file
    std::ifstream file(compiled_path, std::ios::binary);
    if (!file.is_open()) {
        LM.writeLog("TextureLoader - Failed to open compiled file: %s", compiled_path.c_str());
        return nullptr;
    }

    // Read and validate header
    gam300::CompiledResourceHeader header;
    if (!gam300::readCompiledHeader(file, header)) {
        return nullptr;
    }

    // Verify resource type
    if (header.resourceType != static_cast<uint32_t>(gam300::ResourceType::TEXTURE)) {
        LM.writeLog("TextureLoader - Resource type mismatch");
        return nullptr;
    }

    // Read texture-specific header
    gam300::CompiledTextureData texHeader;
    file.read(reinterpret_cast<char*>(&texHeader), sizeof(gam300::CompiledTextureData));

    if (!file) {
        LM.writeLog("TextureLoader - Failed to read texture header");
        return nullptr;
    }

    // Create texture resource
    auto texture = std::make_unique<data_type>();
    texture->width = texHeader.width;
    texture->height = texHeader.height;
    texture->channels = texHeader.channels;

    // Determine OpenGL format
    GLenum internalFormat, format, type;
    if (texHeader.srgb) {
        internalFormat = (texHeader.channels == 4) ? GL_SRGB8_ALPHA8 : GL_SRGB8;
    }
    else {
        internalFormat = (texHeader.channels == 4) ? GL_RGBA8 : GL_RGB8;
    }
    format = (texHeader.channels == 4) ? GL_RGBA : GL_RGB;
    type = GL_UNSIGNED_BYTE;

    // Generate OpenGL texture
    glGenTextures(1, &texture->textureID);
    glBindTexture(GL_TEXTURE_2D, texture->textureID);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
        texHeader.mipLevels > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Read and upload each mip level
    int currentWidth = texHeader.width;
    int currentHeight = texHeader.height;

    for (uint32_t mipLevel = 0; mipLevel < texHeader.mipLevels; ++mipLevel) {
        // Calculate size of this mip level
        size_t mipSize = static_cast<size_t>(currentWidth) * currentHeight * texHeader.channels;

        // Read mip data
        std::vector<unsigned char> mipData(mipSize);
        file.read(reinterpret_cast<char*>(mipData.data()), mipSize);

        if (!file) {
            LM.writeLog("TextureLoader - Failed to read mip level %u", mipLevel);
            glDeleteTextures(1, &texture->textureID);
            return nullptr;
        }

        // Upload to GPU
        glTexImage2D(GL_TEXTURE_2D, mipLevel, internalFormat,
            currentWidth, currentHeight, 0, format, type, mipData.data());

        // Calculate next mip dimensions
        currentWidth = std::max(1, currentWidth / 2);
        currentHeight = std::max(1, currentHeight / 2);
    }

    // Check for OpenGL errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        LM.writeLog("TextureLoader - OpenGL error: 0x%X", error);
        glDeleteTextures(1, &texture->textureID);
        return nullptr;
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    texture->format = texHeader.srgb ? "sRGB" : "RGB";

    LM.writeLog("TextureLoader - Loaded texture GUID: %llX, OpenGL ID: %u, Size: %dx%d, Mips: %u",
        guid.m_Instance.m_Value, texture->textureID,
        texture->width, texture->height, texHeader.mipLevels);

    return texture.release();
}

void xresource::loader<gam300::ResourceGUID::texture_type_guid_v>::Destroy(
    xresource::mgr& /*mgr*/, data_type&& data, const full_guid& guid)
{
    // Delete OpenGL texture
    if (data.textureID != 0) {
        glDeleteTextures(1, &data.textureID);
        LM.writeLog("TextureLoader - Destroyed texture GUID: %llX, OpenGL ID: %u",
            guid.m_Instance.m_Value, data.textureID);
    }

    delete& data;
}


// ========== MESH LOADER IMPLEMENTATION ==========

#if 0
xresource::loader<gam300::ResourceGUID::mesh_type_guid_v>::data_type*
xresource::loader<gam300::ResourceGUID::mesh_type_guid_v>::Load(
    xresource::mgr& mgr, const full_guid& guid)
{
    gam300::ResourceManager* rm = gam300::getResourceManager(mgr);

    // Get compiled file path
    std::string compiled_path = rm->getPaths().getCompiledFilePath(guid, gam300::ResourceType::MESH);

    if (!rm->getPaths().fileExists(compiled_path)) {
        LM.writeLog("MeshLoader - Compiled file not found: %s", compiled_path.c_str());
        return nullptr;
    }

    // Open compiled binary file
    std::ifstream file(compiled_path, std::ios::binary);
    if (!file.is_open()) {
        LM.writeLog("MeshLoader - Failed to open compiled file: %s", compiled_path.c_str());
        return nullptr;
    }

    // Read and validate header
    gam300::CompiledResourceHeader header;
    if (!gam300::readCompiledHeader(file, header)) {
        return nullptr;
    }

    // Verify resource type
    if (header.resourceType != static_cast<uint32_t>(gam300::ResourceType::MESH)) {
        LM.writeLog("MeshLoader - Resource type mismatch");
        return nullptr;
    }

    // Read mesh data counts
    uint32_t vertexCount, indexCount;
    file.read(reinterpret_cast<char*>(&vertexCount), sizeof(uint32_t));
    file.read(reinterpret_cast<char*>(&indexCount), sizeof(uint32_t));

    if (!file) {
        LM.writeLog("MeshLoader - Failed to read mesh counts");
        return nullptr;
    }

    // Create mesh resource
    auto mesh = std::make_unique<data_type>();

    // Read vertex data
    mesh->vertices.resize(vertexCount);
    file.read(reinterpret_cast<char*>(mesh->vertices.data()),
        vertexCount * sizeof(float));

    // Read index data
    mesh->indices.resize(indexCount);
    file.read(reinterpret_cast<char*>(mesh->indices.data()),
        indexCount * sizeof(unsigned int));

    if (!file) {
        LM.writeLog("MeshLoader - Failed to read mesh data");
        return nullptr;
    }

    // Create OpenGL buffers
    glGenVertexArrays(1, &mesh->VAO);
    glGenBuffers(1, &mesh->VBO);
    glGenBuffers(1, &mesh->EBO);

    glBindVertexArray(mesh->VAO);

    // Upload vertex data
    glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);
    glBufferData(GL_ARRAY_BUFFER,
        mesh->vertices.size() * sizeof(float),
        mesh->vertices.data(), GL_STATIC_DRAW);

    // Upload index data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        mesh->indices.size() * sizeof(unsigned int),
        mesh->indices.data(), GL_STATIC_DRAW);

    // Setup vertex attributes (position, normal, texcoord)
    // Assuming interleaved format: pos(3) + normal(3) + uv(2) = 8 floats per vertex
    size_t stride = 8 * sizeof(float);

    // Position attribute (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);

    // Normal attribute (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // TexCoord attribute (location = 2)
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    // Check for OpenGL errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        LM.writeLog("MeshLoader - OpenGL error: 0x%X", error);
        glDeleteVertexArrays(1, &mesh->VAO);
        glDeleteBuffers(1, &mesh->VBO);
        glDeleteBuffers(1, &mesh->EBO);
        return nullptr;
    }

    LM.writeLog("MeshLoader - Loaded mesh GUID: %llX, VAO: %u, Vertices: %u, Indices: %u",
        guid.m_Instance.m_Value, mesh->VAO, vertexCount, indexCount);

    return mesh.release();
}

#endif

// In ResourceLoaders.cpp - Update the MeshLoader::Load function

xresource::loader<gam300::ResourceGUID::mesh_type_guid_v>::data_type*
xresource::loader<gam300::ResourceGUID::mesh_type_guid_v>::Load(
    xresource::mgr& mgr, const full_guid& guid)
{
    gam300::ResourceManager* rm = gam300::getResourceManager(mgr);

    // Get compiled file path
    std::string compiled_path = rm->getPaths().getCompiledFilePath(guid, gam300::ResourceType::MESH);

    if (!rm->getPaths().fileExists(compiled_path)) {
        LM.writeLog("MeshLoader - Compiled file not found: %s", compiled_path.c_str());
        return nullptr;
    }

    // Open compiled binary file
    std::ifstream file(compiled_path, std::ios::binary);
    if (!file.is_open()) {
        LM.writeLog("MeshLoader - Failed to open: %s", compiled_path.c_str());
        return nullptr;
    }

    // Read compiled resource header
    gam300::CompiledResourceHeader header;
    if (!gam300::readCompiledHeader(file, header)) {
        return nullptr;
    }

    // Verify resource type
    if (header.resourceType != static_cast<uint32_t>(gam300::ResourceType::MESH)) {
        LM.writeLog("MeshLoader - Resource type mismatch");
        return nullptr;
    }

    // Read compiled mesh data header
    gam300::CompiledMeshData meshHeader;
    file.read(reinterpret_cast<char*>(&meshHeader), sizeof(meshHeader));
    if (!file) {
        LM.writeLog("MeshLoader - Failed to read mesh header");
        return nullptr;
    }

    // Create mesh resource
    auto mesh = std::make_unique<data_type>();

    // Read separate arrays (matching MeshCompiler output)
    std::vector<glm::vec3> positions(meshHeader.vertexCount);
    std::vector<glm::vec3> normals(meshHeader.vertexCount);
    std::vector<glm::vec3> colors(meshHeader.vertexCount);
    std::vector<glm::vec2> texcoords(meshHeader.vertexCount);

    // Read positions
    file.read(reinterpret_cast<char*>(positions.data()),
        meshHeader.vertexCount * sizeof(glm::vec3));

    // Read normals
    if (meshHeader.hasNormals) {
        file.read(reinterpret_cast<char*>(normals.data()),
            meshHeader.vertexCount * sizeof(glm::vec3));
    }

    // Read colors
    if (meshHeader.hasColors) {
        file.read(reinterpret_cast<char*>(colors.data()),
            meshHeader.vertexCount * sizeof(glm::vec3));
    }

    // Read texcoords
    if (meshHeader.hasTexCoords) {
        file.read(reinterpret_cast<char*>(texcoords.data()),
            meshHeader.vertexCount * sizeof(glm::vec2));
    }

    // Read indices
    mesh->indices.resize(meshHeader.indexCount);
    file.read(reinterpret_cast<char*>(mesh->indices.data()),
        meshHeader.indexCount * sizeof(unsigned int));

    if (!file) {
        LM.writeLog("MeshLoader - Failed to read mesh data");
        return nullptr;
    }

    // Convert to interleaved format for OpenGL
    // Format: pos(3) + normal(3) + color(3) + uv(2) = 11 floats per vertex
    mesh->vertices.resize(meshHeader.vertexCount * 11);

    for (uint32_t i = 0; i < meshHeader.vertexCount; ++i) {
        size_t offset = i * 11;

        // Position (3 floats)
        mesh->vertices[offset + 0] = positions[i].x;
        mesh->vertices[offset + 1] = positions[i].y;
        mesh->vertices[offset + 2] = positions[i].z;

        // Normal (3 floats)
        mesh->vertices[offset + 3] = normals[i].x;
        mesh->vertices[offset + 4] = normals[i].y;
        mesh->vertices[offset + 5] = normals[i].z;

        // Color (3 floats)
        mesh->vertices[offset + 6] = colors[i].x;
        mesh->vertices[offset + 7] = colors[i].y;
        mesh->vertices[offset + 8] = colors[i].z;

        // TexCoord (2 floats)
        mesh->vertices[offset + 9] = texcoords[i].x;
        mesh->vertices[offset + 10] = texcoords[i].y;
    }

    // Create OpenGL buffers
    glGenVertexArrays(1, &mesh->VAO);
    glGenBuffers(1, &mesh->VBO);
    glGenBuffers(1, &mesh->EBO);

    glBindVertexArray(mesh->VAO);

    // Upload interleaved vertex data
    glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);
    glBufferData(GL_ARRAY_BUFFER,
        mesh->vertices.size() * sizeof(float),
        mesh->vertices.data(), GL_STATIC_DRAW);

    // Upload index data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        mesh->indices.size() * sizeof(unsigned int),
        mesh->indices.data(), GL_STATIC_DRAW);

    // Setup vertex attributes - interleaved format
    size_t stride = 11 * sizeof(float);

    // Position attribute (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);

    // Normal attribute (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Color attribute (location = 2)
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // TexCoord attribute (location = 3)
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, stride, (void*)(9 * sizeof(float)));
    glEnableVertexAttribArray(3);

    glBindVertexArray(0);

    // Check for OpenGL errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        LM.writeLog("MeshLoader - OpenGL error: 0x%X", error);
        glDeleteVertexArrays(1, &mesh->VAO);
        glDeleteBuffers(1, &mesh->VBO);
        glDeleteBuffers(1, &mesh->EBO);
        return nullptr;
    }

    LM.writeLog("MeshLoader - Loaded mesh GUID: %llX, VAO: %u, Vertices: %u, Indices: %u",
        guid.m_Instance.m_Value, mesh->VAO, meshHeader.vertexCount, meshHeader.indexCount);

    return mesh.release();
}


void xresource::loader<gam300::ResourceGUID::mesh_type_guid_v>::Destroy(
    xresource::mgr& /*mgr*/, data_type&& data, const full_guid& guid)
{
    // Delete OpenGL buffers
    if (data.VAO != 0) {
        glDeleteVertexArrays(1, &data.VAO);
    }
    if (data.VBO != 0) {
        glDeleteBuffers(1, &data.VBO);
    }
    if (data.EBO != 0) {
        glDeleteBuffers(1, &data.EBO);
    }

    LM.writeLog("MeshLoader - Destroyed mesh GUID: %llX", guid.m_Instance.m_Value);

    delete& data;
}


// ========== MATERIAL LOADER IMPLEMENTATION ==========

xresource::loader<gam300::ResourceGUID::material_type_guid_v>::data_type*
xresource::loader<gam300::ResourceGUID::material_type_guid_v>::Load(
    xresource::mgr& mgr, const full_guid& guid)
{
    gam300::ResourceManager* rm = gam300::getResourceManager(mgr);

    // Get compiled file path
    std::string compiled_path = rm->getPaths().getCompiledFilePath(guid, gam300::ResourceType::MATERIAL);

    if (!rm->getPaths().fileExists(compiled_path)) {
        LM.writeLog("MaterialLoader - Compiled file not found: %s", compiled_path.c_str());
        return nullptr;
    }

    // Open compiled binary file
    std::ifstream file(compiled_path, std::ios::binary);
    if (!file.is_open()) {
        LM.writeLog("MaterialLoader - Failed to open: %s", compiled_path.c_str());
        return nullptr;
    }

    // Read header
    gam300::CompiledResourceHeader header;
    if (!gam300::readCompiledHeader(file, header)) {
        return nullptr;
    }

    // Create material resource
    auto material = std::make_unique<data_type>();

    // Read material properties
    file.read(reinterpret_cast<char*>(&material->diffuseTexture), sizeof(xresource::full_guid));
    file.read(reinterpret_cast<char*>(&material->normalTexture), sizeof(xresource::full_guid));
    file.read(reinterpret_cast<char*>(&material->specularTexture), sizeof(xresource::full_guid));
    file.read(reinterpret_cast<char*>(&material->shininess), sizeof(float));
    file.read(reinterpret_cast<char*>(&material->opacity), sizeof(float));
    file.read(reinterpret_cast<char*>(&material->doubleSided), sizeof(bool));

    // Read shader name length and string
    uint32_t nameLength;
    file.read(reinterpret_cast<char*>(&nameLength), sizeof(uint32_t));
    material->shaderName.resize(nameLength);
    file.read(material->shaderName.data(), nameLength);

    if (!file) {
        LM.writeLog("MaterialLoader - Failed to read material data");
        return nullptr;
    }

    LM.writeLog("MaterialLoader - Loaded material GUID: %llX", guid.m_Instance.m_Value);

    return material.release();
}

void xresource::loader<gam300::ResourceGUID::material_type_guid_v>::Destroy(
    xresource::mgr& /*mgr*/, data_type&& data, const full_guid& guid)
{
    LM.writeLog("MaterialLoader - Destroyed material GUID: %llX", guid.m_Instance.m_Value);
    delete& data;
}


// ========== AUDIO LOADER IMPLEMENTATION ==========

xresource::loader<gam300::ResourceGUID::audio_type_guid_v>::data_type*
xresource::loader<gam300::ResourceGUID::audio_type_guid_v>::Load(
    xresource::mgr& mgr, const full_guid& guid)
{
    gam300::ResourceManager* rm = gam300::getResourceManager(mgr);

    std::string compiled_path = rm->getPaths().getCompiledFilePath(guid, gam300::ResourceType::AUDIO);

    if (!rm->getPaths().fileExists(compiled_path)) {
        LM.writeLog("AudioLoader - Compiled file not found: %s", compiled_path.c_str());
        return nullptr;
    }

    std::ifstream file(compiled_path, std::ios::binary);
    if (!file.is_open()) {
        LM.writeLog("AudioLoader - Failed to open: %s", compiled_path.c_str());
        return nullptr;
    }

    gam300::CompiledResourceHeader header;
    if (!gam300::readCompiledHeader(file, header)) {
        return nullptr;
    }

    auto audio = std::make_unique<data_type>();

    // Read audio properties
    file.read(reinterpret_cast<char*>(&audio->sampleRate), sizeof(int));
    file.read(reinterpret_cast<char*>(&audio->channels), sizeof(int));
    file.read(reinterpret_cast<char*>(&audio->bitDepth), sizeof(int));

    // Read audio data size and data
    uint32_t dataSize;
    file.read(reinterpret_cast<char*>(&dataSize), sizeof(uint32_t));
    audio->audioData.resize(dataSize);
    file.read(audio->audioData.data(), dataSize);

    if (!file) {
        LM.writeLog("AudioLoader - Failed to read audio data");
        return nullptr;
    }

    // TODO: Create OpenAL buffer if using OpenAL
    // For now, just keep data in memory

    LM.writeLog("AudioLoader - Loaded audio GUID: %llX, Size: %u bytes",
        guid.m_Instance.m_Value, dataSize);

    return audio.release();
}

void xresource::loader<gam300::ResourceGUID::audio_type_guid_v>::Destroy(
    xresource::mgr& /*mgr*/, data_type&& data, const full_guid& guid)
{
    // TODO: Delete OpenAL buffer if created
    LM.writeLog("AudioLoader - Destroyed audio GUID: %llX", guid.m_Instance.m_Value);
    delete& data;
}


// ========== SHADER LOADER IMPLEMENTATION ==========

xresource::loader<gam300::ResourceGUID::shader_type_guid_v>::data_type*
xresource::loader<gam300::ResourceGUID::shader_type_guid_v>::Load(
    xresource::mgr& mgr, const full_guid& guid)
{
    gam300::ResourceManager* rm = gam300::getResourceManager(mgr);

    std::string compiled_path = rm->getPaths().getCompiledFilePath(guid, gam300::ResourceType::SHADER);

    if (!rm->getPaths().fileExists(compiled_path)) {
        LM.writeLog("ShaderLoader - Compiled file not found: %s", compiled_path.c_str());
        return nullptr;
    }

    std::ifstream file(compiled_path, std::ios::binary);
    if (!file.is_open()) {
        LM.writeLog("ShaderLoader - Failed to open: %s", compiled_path.c_str());
        return nullptr;
    }

    gam300::CompiledResourceHeader header;
    if (!gam300::readCompiledHeader(file, header)) {
        return nullptr;
    }

    auto shader = std::make_unique<data_type>();

    // Read shader source lengths and sources
    uint32_t vertLength, fragLength, geomLength;
    file.read(reinterpret_cast<char*>(&vertLength), sizeof(uint32_t));
    file.read(reinterpret_cast<char*>(&fragLength), sizeof(uint32_t));
    file.read(reinterpret_cast<char*>(&geomLength), sizeof(uint32_t));

    if (vertLength > 0) {
        shader->vertexSource.resize(vertLength);
        file.read(shader->vertexSource.data(), vertLength);
    }

    if (fragLength > 0) {
        shader->fragmentSource.resize(fragLength);
        file.read(shader->fragmentSource.data(), fragLength);
    }

    if (geomLength > 0) {
        shader->geometrySource.resize(geomLength);
        file.read(shader->geometrySource.data(), geomLength);
    }

    if (!file) {
        LM.writeLog("ShaderLoader - Failed to read shader sources");
        return nullptr;
    }

    // TODO: Compile and link OpenGL shader program
    // For now, just keep sources in memory

    LM.writeLog("ShaderLoader - Loaded shader GUID: %llX", guid.m_Instance.m_Value);

    return shader.release();
}

void xresource::loader<gam300::ResourceGUID::shader_type_guid_v>::Destroy(
    xresource::mgr& /*mgr*/, data_type&& data, const full_guid& guid)
{
    // TODO: Delete OpenGL shader program if created
    if (data.programID != 0) {
        glDeleteProgram(data.programID);
    }

    LM.writeLog("ShaderLoader - Destroyed shader GUID: %llX", guid.m_Instance.m_Value);
    delete& data;
}