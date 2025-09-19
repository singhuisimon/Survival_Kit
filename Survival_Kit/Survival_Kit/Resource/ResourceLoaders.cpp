/**
 * @file ResourceLoaders.cpp
 * @brief Implementation of xresource_mgr loaders for different resource types.
 * @details Contains the actual loading and destruction logic for each resource type.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#include "ResourceData.h"
#include "../Manager/ResourceManager.h"
#include "../Manager/LogManager.h"
#include <fstream>
#include <memory>

namespace gam300 {

    // Helper function to get ResourceManager from xresource::mgr
    ResourceManager* getResourceManager(xresource::mgr& mgr) {
        return &mgr.getUserData<ResourceManager>();
    }

} // namespace gam300

// ========== TEXTURE LOADER IMPLEMENTATION ==========

xresource::loader<gam300::ResourceGUID::texture_type_guid_v>::data_type*
xresource::loader<gam300::ResourceGUID::texture_type_guid_v>::Load(xresource::mgr& mgr, const full_guid& guid) {

    gam300::ResourceManager* rm = gam300::getResourceManager(mgr);

    // Get resource properties from descriptor
    auto* properties = rm->getResourceProperties(guid, gam300::ResourceType::TEXTURE);
    if (!properties) {
       LM.writeLog("TextureLoader::Load() - Failed to get properties for GUID: %llX", guid.m_Instance.m_Value);
        return nullptr;
    }

    auto* tex_props = gam300::cast_properties<gam300::TextureProperties>(properties);
    if (!tex_props) {
        LM.writeLog("TextureLoader::Load() - Invalid texture properties for GUID: %llX", guid.m_Instance.m_Value);
        return nullptr;
    }

    // Get the intermediate file path
    std::string intermediate_path = rm->getPaths().getIntermediateFilePath(tex_props->intermediateFilePath);

    if (!rm->getPaths().fileExists(intermediate_path)) {
        LM.writeLog("TextureLoader::Load() - Intermediate file not found: %s", intermediate_path.c_str());
        return nullptr;
    }

    // Create texture resource
    auto texture = std::make_unique<data_type>();

    // TODO: Implement actual texture loading using stb_image or similar
    // For now, create a placeholder texture
    texture->width = tex_props->maxWidth;
    texture->height = tex_props->maxHeight;
    texture->channels = 4; // RGBA
    texture->textureID = 0; // TODO: Generate OpenGL texture
    texture->format = tex_props->compressionFormat;

    LM.writeLog("TextureLoader::Load() - Loaded texture: %s (%dx%d)",
        tex_props->resourceName.c_str(), texture->width, texture->height);

    return texture.release();
}

void xresource::loader<gam300::ResourceGUID::texture_type_guid_v>::Destroy(xresource::mgr& /*mgr*/, data_type&& data, const full_guid& guid) {
   LM.writeLog("TextureLoader::Destroy() - Destroying texture GUID: %llX", guid.m_Instance.m_Value);
    delete& data;
}

// ========== MESH LOADER IMPLEMENTATION ==========

xresource::loader<gam300::ResourceGUID::mesh_type_guid_v>::data_type*
xresource::loader<gam300::ResourceGUID::mesh_type_guid_v>::Load(xresource::mgr& mgr, const full_guid& guid) {

    gam300::ResourceManager* rm = gam300::getResourceManager(mgr);

    auto* properties = rm->getResourceProperties(guid, gam300::ResourceType::MESH);
    if (!properties) {
       LM.writeLog("MeshLoader::Load() - Failed to get properties for GUID: %llX", guid.m_Instance.m_Value);
        return nullptr;
    }

    auto* mesh_props = gam300::cast_properties<gam300::MeshProperties>(properties);
    if (!mesh_props) {
       LM.writeLog("MeshLoader::Load() - Invalid mesh properties for GUID: %llX", guid.m_Instance.m_Value);
        return nullptr;
    }

    // Get the intermediate file path
    std::string intermediate_path = rm->getPaths().getIntermediateFilePath(mesh_props->intermediateFilePath);

    if (!rm->getPaths().fileExists(intermediate_path)) {
        LM.writeLog("MeshLoader::Load() - Intermediate file not found: %s", intermediate_path.c_str());
        return nullptr;
    }

    // Create mesh resource
    auto mesh = std::make_unique<data_type>();

    // TODO: Implement actual mesh loading using Assimp or similar
    // For now, create a placeholder triangle
    mesh->vertices = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f
    };
    mesh->indices = { 0, 1, 2 };
    mesh->VAO = 0; // TODO: Generate OpenGL VAO
    mesh->VBO = 0; // TODO: Generate OpenGL VBO
    mesh->EBO = 0; // TODO: Generate OpenGL EBO

    LM.writeLog("MeshLoader::Load() - Loaded mesh: %s (%zu vertices)",
        mesh_props->resourceName.c_str(), mesh->vertices.size() / 3);

    return mesh.release();
}

void xresource::loader<gam300::ResourceGUID::mesh_type_guid_v>::Destroy(xresource::mgr& /*mgr*/, data_type&& data, const full_guid& guid) {
   LM.writeLog("MeshLoader::Destroy() - Destroying mesh GUID: %llX", guid.m_Instance.m_Value);
    delete& data;
}

// ========== MATERIAL LOADER IMPLEMENTATION ==========

xresource::loader<gam300::ResourceGUID::material_type_guid_v>::data_type*
xresource::loader<gam300::ResourceGUID::material_type_guid_v>::Load(xresource::mgr& mgr, const full_guid& guid) {

    gam300::ResourceManager* rm = gam300::getResourceManager(mgr);

    auto* properties = rm->getResourceProperties(guid, gam300::ResourceType::MATERIAL);
    if (!properties) {
        LM.writeLog("MaterialLoader::Load() - Failed to get properties for GUID: %llX", guid.m_Instance.m_Value);
        return nullptr;
    }

    auto* mat_props = gam300::cast_properties<gam300::MaterialProperties>(properties);
    if (!mat_props) {
       LM.writeLog("MaterialLoader::Load() - Invalid material properties for GUID: %llX", guid.m_Instance.m_Value);
        return nullptr;
    }

    // Create material resource
    auto material = std::make_unique<data_type>();

    material->shaderName = mat_props->shaderName;
    material->diffuseTexture = mat_props->diffuseTexture;
    material->normalTexture = mat_props->normalTexture;
    material->specularTexture = mat_props->specularTexture;
    material->shininess = mat_props->shininess;
    material->opacity = mat_props->opacity;
    material->doubleSided = mat_props->doubleSided;

    LM.writeLog("MaterialLoader::Load() - Loaded material: %s (shader: %s)",
        mat_props->resourceName.c_str(), material->shaderName.c_str());

    return material.release();
}

void xresource::loader<gam300::ResourceGUID::material_type_guid_v>::Destroy(xresource::mgr& /*mgr*/, data_type&& data, const full_guid& guid) {
    LM.writeLog("MaterialLoader::Destroy() - Destroying material GUID: %llX", guid.m_Instance.m_Value);
    delete& data;
}

// ========== AUDIO LOADER IMPLEMENTATION ==========

xresource::loader<gam300::ResourceGUID::audio_type_guid_v>::data_type*
xresource::loader<gam300::ResourceGUID::audio_type_guid_v>::Load(xresource::mgr& mgr, const full_guid& guid) {

    gam300::ResourceManager* rm = gam300::getResourceManager(mgr);

    auto* properties = rm->getResourceProperties(guid, gam300::ResourceType::AUDIO);
    if (!properties) {
       LM.writeLog("AudioLoader::Load() - Failed to get properties for GUID: %llX", guid.m_Instance.m_Value);
        return nullptr;
    }

    auto* audio_props = gam300::cast_properties<gam300::AudioProperties>(properties);
    if (!audio_props) {
      LM.writeLog("AudioLoader::Load() - Invalid audio properties for GUID: %llX", guid.m_Instance.m_Value);
        return nullptr;
    }

    // Get the intermediate file path
    std::string intermediate_path = rm->getPaths().getIntermediateFilePath(audio_props->intermediateFilePath);

    if (!rm->getPaths().fileExists(intermediate_path)) {
        LM.writeLog("AudioLoader::Load() - Intermediate file not found: %s", intermediate_path.c_str());
        return nullptr;
    }

    // Create audio resource
    auto audio = std::make_unique<data_type>();

    // TODO: Implement actual audio loading using OpenAL or similar
    // For now, create placeholder audio data
    audio->sampleRate = audio_props->sampleRate;
    audio->channels = audio_props->channels;
    audio->bitDepth = audio_props->bitDepth;
    audio->bufferID = 0; // TODO: Generate OpenAL buffer

    // Create some placeholder audio data
    size_t dataSize = audio->sampleRate * audio->channels * (audio->bitDepth / 8);
    audio->audioData.resize(dataSize, 0);

   LM.writeLog("AudioLoader::Load() - Loaded audio: %s (%d Hz, %d channels)",
        audio_props->resourceName.c_str(), audio->sampleRate, audio->channels);

    return audio.release();
}

void xresource::loader<gam300::ResourceGUID::audio_type_guid_v>::Destroy(xresource::mgr& /*mgr*/, data_type&& data, const full_guid& guid) {
   LM.writeLog("AudioLoader::Destroy() - Destroying audio GUID: %llX", guid.m_Instance.m_Value);
    delete& data;
}

// ========== SHADER LOADER IMPLEMENTATION ==========

xresource::loader<gam300::ResourceGUID::shader_type_guid_v>::data_type*
xresource::loader<gam300::ResourceGUID::shader_type_guid_v>::Load(xresource::mgr& mgr, const full_guid& guid) {

    gam300::ResourceManager* rm = gam300::getResourceManager(mgr);

    auto* properties = rm->getResourceProperties(guid, gam300::ResourceType::SHADER);
    if (!properties) {
       LM.writeLog("ShaderLoader::Load() - Failed to get properties for GUID: %llX", guid.m_Instance.m_Value);
        return nullptr;
    }

    auto* shader_props = gam300::cast_properties<gam300::ShaderProperties>(properties);
    if (!shader_props) {
       LM.writeLog("ShaderLoader::Load() - Invalid shader properties for GUID: %llX", guid.m_Instance.m_Value);
        return nullptr;
    }

    // Create shader resource
    auto shader = std::make_unique<data_type>();

    // Load shader source files
    if (!shader_props->vertexShaderPath.empty()) {
        std::string vertex_path = rm->getPaths().getIntermediateFilePath(shader_props->vertexShaderPath);
        if (rm->getPaths().fileExists(vertex_path)) {
            // TODO: Read vertex shader source from file
            shader->vertexSource = "// Vertex shader placeholder\n";
        }
    }

    if (!shader_props->fragmentShaderPath.empty()) {
        std::string fragment_path = rm->getPaths().getIntermediateFilePath(shader_props->fragmentShaderPath);
        if (rm->getPaths().fileExists(fragment_path)) {
            // TODO: Read fragment shader source from file
            shader->fragmentSource = "// Fragment shader placeholder\n";
        }
    }

    if (!shader_props->geometryShaderPath.empty()) {
        std::string geometry_path = rm->getPaths().getIntermediateFilePath(shader_props->geometryShaderPath);
        if (rm->getPaths().fileExists(geometry_path)) {
            // TODO: Read geometry shader source from file
            shader->geometrySource = "// Geometry shader placeholder\n";
        }
    }

    // TODO: Compile and link OpenGL shader program
    shader->programID = 0; // TODO: Generate OpenGL shader program

    LM.writeLog("ShaderLoader::Load() - Loaded shader: %s",
        shader_props->resourceName.c_str());

    return shader.release();
}

void xresource::loader<gam300::ResourceGUID::shader_type_guid_v>::Destroy(xresource::mgr& /*mgr*/, data_type&& data, const full_guid& guid) {
    LM.writeLog("ShaderLoader::Destroy() - Destroying shader GUID: %llX", guid.m_Instance.m_Value);
    delete& data;
}