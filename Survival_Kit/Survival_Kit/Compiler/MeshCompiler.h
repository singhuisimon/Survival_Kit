/**
 * @file MeshCompiler.h
 * @brief Mesh resource compiler.
 * @details Compiles intermediate mesh data to optimized binary format.
 * @author Wai Lwin Thit
 * @date October 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once
#ifndef __MESH_COMPILER_H__
#define __MESH_COMPILER_H__

#include "ResourceCompiler.h"
// For access to vector types
#include <glm-0.9.9.8/glm/glm.hpp>



namespace gam300 {

    struct CompiledMeshData {
        uint32_t vertexCount;
        uint32_t indexCount;

        uint32_t hasPositions; //always 1
        uint32_t hasColors;

        uint32_t reserved[2];

        uint32_t vertexStride; //bytes per vertex (32 for pos+normal+uv)
        uint32_t hasNormals; //1 if normals present
        uint32_t hasTexCoords; //1 if UVs present
    };

    //mesh vertex
    struct MeshInfo {
        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec3> colors;
        std::vector<glm::vec2> texCoords;
        std::vector<uint32_t> indices;

    };

    class MeshCompiler : public IResourceCompiler {
    public:
        CompileResult compile(
            const std::string& intermediatePath,
            const ResourceProperties* properties,
            ResourcePaths& paths,
            const xresource::full_guid& guid
        ) override;

        ResourceType getResourceType() const override {
            return ResourceType::MESH;
        }

    private:

        //mesh loading from FBX
        bool loadFBXMesh(const std::string& path,
            MeshInfo& meshInfo);

        //mesh optimization funcitons
        void scaleMesh(MeshInfo& meshInfo, float scale);
        void generateNormals(MeshInfo& meshInfo);
        void flipUVs(MeshInfo& meshInfo);
        void removeDegenerate(MeshInfo& meshInfo);
        void weldVertices(MeshInfo& meshInfo, float threshold);

        //write binary file for mesh
        bool writeCompiledMesh(const std::string& outputPath,
            const CompiledMeshData& header,
            const MeshInfo& meshInfo,
            const xresource::full_guid& guid);


    };

} // namespace gam300

#endif // __MESH_COMPILER_H__