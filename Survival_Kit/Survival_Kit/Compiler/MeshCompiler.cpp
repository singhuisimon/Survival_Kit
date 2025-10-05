/**
 * @file MeshCompiler.cpp
 * @brief Implementation of mesh compilation.
 * @details Handles mesh optimization and binary serialization for runtime loading.
 * @author Wai Lwin Thit
 * @date October 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#include "MeshCompiler.h"
#include <fstream>
#include <filesystem>

//to load FBX mesh data
#include "../OpenFBX/ofbx.h"


namespace fs = std::filesystem;

namespace gam300 {

    CompileResult MeshCompiler::compile(
        const std::string& intermediatePath,
        [[maybe_unused]] const ResourceProperties* properties,
        ResourcePaths& paths,
        const xresource::full_guid& guid
    ) {
        CompileResult result;

        //check for source file path
        if (!fs::exists(intermediatePath)) {
            result.error = "Intermediate file not found: " + intermediatePath;
            LM.writeLog("MeshCompiler - %s", result.error.c_str());
            return result;
        }

        //get the original file size
        result.originalSize = paths.getFileSize(intermediatePath);

        //cast mesh properties 
        const MeshProperties* meshProps = cast_properties<MeshProperties>(properties);
        if (!meshProps) {
            result.error = "Invalid mesh properties";
            LM.writeLog("MeshCompiler - Invalid properties for compilation");
            return result;

        }

        LM.writeLog("MeshCompiler - Compiling: %s", intermediatePath.c_str());
        LM.writeLog("MeshCompiler - Properties: scale=%.2f, optimize=%d, normals=%d",
            meshProps->scaleFactor,
            meshProps->optimizeVertices,
            meshProps->generateNormals);

        //load mesh data from FBX using struct
        MeshInfo meshData;

        if (!loadFBXMesh(intermediatePath, meshData)) {
            result.error = "Failed to load FBX mesh data";
            LM.writeLog("MeshCompiler - %s", result.error.c_str());
            return result;
        }

        if (meshData.positions.empty()) {
            result.error = "Failed to load FBX mesh data";
            LM.writeLog("MeshCompiler - %s", result.error.c_str());
            return result;
        }

        LM.writeLog("MeshCompiler - Loaded: %zu vertices, %zu indices",
            meshData.positions.size(), meshData.indices.size());

        //apply mesh processing based on properties
        if (meshProps->scaleFactor != 1.0f) {
            scaleMesh(meshData, meshProps->scaleFactor);
            LM.writeLog("MeshCompiler - Applied scale: %.2f", meshProps->scaleFactor);
        }

        if (meshProps->generateNormals) {
            generateNormals(meshData);
            LM.writeLog("MeshCompiler - Generated normals");
        }

        if (meshProps->flipUVs) {
            flipUVs(meshData);
            LM.writeLog("MeshCompiler - Flipped UVs");
        }

        if (meshProps->removeDegenerate) {
            size_t beforeCount = meshData.indices.size();
            removeDegenerate(meshData);
            LM.writeLog("MeshCompiler - Removed degenerate triangle: %zu -> %zu",
                beforeCount / 3, meshData.indices.size() / 3);
        }

        if (meshProps->weldVertices) {
            size_t beforeCount = meshData.positions.size();
            weldVertices(meshData, meshProps->weldThreshold);
            LM.writeLog("MeshCompiler - Welded vertices: %zu -> %zu (threshold: %.5f",
                beforeCount, meshData.positions.size(), meshProps->weldThreshold);
        }

        //prepare compiled mesh header
        CompiledMeshData header{};
        header.vertexCount = static_cast<uint32_t>(meshData.positions.size());
        header.indexCount = static_cast<uint32_t>(meshData.indices.size());
        header.hasPositions = 1;
        header.hasNormals = meshData.normals.empty() ? 0 : 1;
        header.hasColors = meshData.colors.empty() ? 0 : 1;
        header.hasTexCoords = meshData.texCoords.empty() ? 0 : 1;

        //compiled path and directory
        std::string compiledPath = paths.getCompiledFilePath(guid, ResourceType::MESH);
        std::string dir = std::filesystem::path(compiledPath).parent_path().string();

        if (!paths.createDirectoryIfNotExists(dir)) {
            result.error = "Failed to create output directory: " + dir;
            LM.writeLog("MeshCompiler - %s", result.error.c_str());
            return result;
        }

        if (!writeCompiledMesh(compiledPath, header, meshData, guid)) {
            result.error = "Failed to write compiled mesh";
            LM.writeLog("MeshCompiler - %s", result.error.c_str());
            return result;
        }

        //success! 
        result.success = true;
        result.compiledPath = compiledPath;
        result.compiledSize = paths.getFileSize(compiledPath);
        result.compressionRatio = static_cast<float>(result.compiledSize) /
            static_cast<float>(result.originalSize);

        LM.writeLog("MeshCompiler - Success! %s (%.2f KB -> %.2f KB)",
            compiledPath.c_str(),
            result.originalSize / 1024.0f,
            result.compiledSize / 1024.0f);

        result.info = "Vertices: " + std::to_string(header.vertexCount) +
            ", Indices: " + std::to_string(header.indexCount);


        return result;
    }

    //need to implement load FBX mesh
    bool MeshCompiler::loadFBXMesh(const std::string& path, MeshInfo& meshData) {
        //load file into memory
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            LM.writeLog("MeshCompiler - Failed to open FBX file: %s", path.c_str());
            return false;
        }

        std::streamsize file_size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<ofbx::u8> content(file_size);
        if (!file.read(reinterpret_cast<char*>(content.data()), file_size)) {
            LM.writeLog("MeshCompiler - Failed to read FBX file: %s", path.c_str());
            return false;
        }

        file.close();

        //load fbx using OpenFBX
        ofbx::IScene* scene = ofbx::load(content.data(), file_size, (ofbx::u16)ofbx::LoadFlags::NONE);


        if (!scene) {
            LM.writeLog("MeshCompiler - Failed to parse FBX: %s (Error: %s)", path.c_str(), ofbx::getError());
            return false;
        }

        LM.writeLog("MeshCompiler - FBX loaded: %d meshes found", scene->getMeshCount());

        //process all meshes in the scene
        int meshCount = scene->getMeshCount();
        for (int mesh_idx = 0; mesh_idx < meshCount; mesh_idx++) {
            const ofbx::Mesh* mesh = scene->getMesh(mesh_idx);
            const ofbx::GeometryData& geom = mesh->getGeometryData();

            LM.writeLog("MeshCompiler - Processing mesh %d: %s",
                mesh_idx, mesh->name);

            size_t vertex_offset = meshData.positions.size();

            //get vertex attributes
            ofbx::Vec3Attributes positions = geom.getPositions();
            ofbx::Vec3Attributes normals = geom.getNormals();
            ofbx::Vec2Attributes uvs = geom.getUVs();

            //process each partition (submesh with same material)
            for (int partition_idx = 0; partition_idx < geom.getPartitionCount(); ++partition_idx) {
                const ofbx::GeometryPartition& partition = geom.getPartition(partition_idx);

                //process each polygon in the partition
                for (int polygon_idx = 0; polygon_idx < partition.polygon_count; ++polygon_idx) {
                    const ofbx::GeometryPartition::Polygon& polygon = partition.polygons[polygon_idx];

                    //extract vertices for this polygon
                    for (int i = polygon.from_vertex; i < polygon.from_vertex + polygon.vertex_count; ++i) {
                        //position 
                        ofbx::Vec3 pos = positions.get(i);
                        meshData.positions.push_back(glm::vec3(
                            static_cast<float>(pos.x),
                            static_cast<float>(pos.y),
                            static_cast<float>(pos.z)
                        ));

                        //normal if available
                        if (normals.values != nullptr) {
                            ofbx::Vec3 normal = normals.get(i);
                            meshData.normals.push_back(glm::vec3(
                                static_cast<float>(normal.x),
                                static_cast<float>(normal.y),
                                static_cast<float>(normal.z)
                            ));
                        }
                        else {
                            meshData.normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                        }

                        // Color (default gray)
                        meshData.colors.push_back(glm::vec3(0.5f));

                        // UV coordinates (if available)
                        if (uvs.values != nullptr) {
                            ofbx::Vec2 uv = uvs.get(i);
                            meshData.texCoords.push_back(glm::vec2(
                                static_cast<float>(uv.x),
                                static_cast<float>(uv.y)
                            ));
                        }
                        else {
                            meshData.texCoords.push_back(glm::vec2(0.0f, 0.0f));
                        }
                    }

                    //triangulate the polygon
                    if (polygon.vertex_count == 3) {
                        //already a triangle
                        meshData.indices.push_back(static_cast<uint32_t>(vertex_offset + polygon.from_vertex + 0));
                        meshData.indices.push_back(static_cast<uint32_t>(vertex_offset + polygon.from_vertex + 1));
                        meshData.indices.push_back(static_cast<uint32_t>(vertex_offset + polygon.from_vertex + 2));

                    }
                    else if (polygon.vertex_count == 4) {
                        // Quad - split into two triangles
                        meshData.indices.push_back(static_cast<uint32_t>(vertex_offset + polygon.from_vertex + 0));
                        meshData.indices.push_back(static_cast<uint32_t>(vertex_offset + polygon.from_vertex + 1));
                        meshData.indices.push_back(static_cast<uint32_t>(vertex_offset + polygon.from_vertex + 2));

                        meshData.indices.push_back(static_cast<uint32_t>(vertex_offset + polygon.from_vertex + 0));
                        meshData.indices.push_back(static_cast<uint32_t>(vertex_offset + polygon.from_vertex + 2));
                        meshData.indices.push_back(static_cast<uint32_t>(vertex_offset + polygon.from_vertex + 3));

                    }
                    else if (polygon.vertex_count > 4) {
                        // N-gon - use OpenFBX triangulate function
                        std::vector<int> tri_indices(polygon.vertex_count * 3);
                        ofbx::u32 tri_count = ofbx::triangulate(geom, polygon, tri_indices.data());

                        for (ofbx::u32 t = 0; t < tri_count; ++t) {
                            meshData.indices.push_back(static_cast<uint32_t>(vertex_offset + tri_indices[t]));
                        }
                    }
                }
            }

            vertex_offset = meshData.positions.size();
        }

        //clean up
        scene->destroy();

        LM.writeLog("MeshCompiler - FBX parsing complete: %zu positions, %zu normals, %zu colors, %zu texCoords, %zu indices",
            meshData.positions.size(), meshData.normals.size(), meshData.colors.size(),
            meshData.texCoords.size(), meshData.indices.size());

        return !meshData.positions.empty();

    }

    void MeshCompiler::scaleMesh(MeshInfo& meshInfo, float scale) {
        for (auto& pos : meshInfo.positions) {
            pos *= scale;
        }
    }

    void MeshCompiler::generateNormals(MeshInfo& meshInfo) {

        //resize normals to match positions
        meshInfo.normals.resize(meshInfo.positions.size(), glm::vec3(0.0f));

        //calculate face normals and accumulate
        for (size_t i = 0; i < meshInfo.indices.size(); i += 3) {
            uint32_t i0 = meshInfo.indices[i];
            uint32_t i1 = meshInfo.indices[i + 1];
            uint32_t i2 = meshInfo.indices[i + 2];

            glm::vec3 edge1 = meshInfo.positions[i1] - meshInfo.positions[i0];
            glm::vec3 edge2 = meshInfo.positions[i2] - meshInfo.positions[i0];
            glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));

            meshInfo.normals[i0] += faceNormal;
            meshInfo.normals[i1] += faceNormal;
            meshInfo.normals[i2] += faceNormal;

        }

        //normalize accumulated normals
        for (auto& normal : meshInfo.normals) {
            if (glm::length(normal) > 0.0001f) {
                normal = glm::normalize(normal);
            }
        }
    }

    void MeshCompiler::flipUVs(MeshInfo& meshInfo) {
        for (auto& uv : meshInfo.texCoords) {
            uv.y = 1.0f - uv.y;
        }
    }

    void MeshCompiler::removeDegenerate(MeshInfo& meshData) {
        std::vector<uint32_t> validIndices;

        for (size_t i = 0; i < meshData.indices.size(); i += 3) {
            uint32_t i0 = meshData.indices[i];
            uint32_t i1 = meshData.indices[i + 1];
            uint32_t i2 = meshData.indices[i + 2];

            if (i0 != i1 && i1 != i2 && i2 != i0) {
                validIndices.push_back(i0);
                validIndices.push_back(i1);
                validIndices.push_back(i2);
            }
        }

        meshData.indices = std::move(validIndices);
    }

    void MeshCompiler::weldVertices(MeshInfo& meshData, float threshold) {
        std::vector<glm::vec3> uniquePos;
        std::vector<glm::vec3> uniqueNormals;
        std::vector<glm::vec3> uniqueColors;
        std::vector<glm::vec2> uniqueTexcoords;
        std::vector<uint32_t> remap(meshData.positions.size());

        float thresholdSq = threshold * threshold;

        for (size_t i = 0; i < meshData.positions.size(); ++i) {
            bool found = false;

            for (size_t j = 0; j < uniquePos.size(); ++j) {
                glm::vec3 diff = meshData.positions[i] - uniquePos[j];
                float distSq = glm::dot(diff, diff);

                if (distSq < thresholdSq) {
                    remap[i] = static_cast<uint32_t>(j);
                    found = true;
                    break;
                }
            }

            if (!found) {
                remap[i] = static_cast<uint32_t>(uniquePos.size());
                uniquePos.push_back(meshData.positions[i]);
                if (i < meshData.normals.size()) uniqueNormals.push_back(meshData.normals[i]);
                if (i < meshData.colors.size()) uniqueColors.push_back(meshData.colors[i]);
                if (i < meshData.texCoords.size()) uniqueTexcoords.push_back(meshData.texCoords[i]);
            }
        }

        // Remap indices
        for (auto& index : meshData.indices) {
            index = remap[index];
        }

        meshData.positions = std::move(uniquePos);
        meshData.normals = std::move(uniqueNormals);
        meshData.colors = std::move(uniqueColors);
        meshData.texCoords = std::move(uniqueTexcoords);
    }

    //write compiled mesh to binary file
    bool MeshCompiler::writeCompiledMesh(const std::string& outputPath,
        const CompiledMeshData& header,
        const MeshInfo& meshData,
        const xresource::full_guid& guid) {

        std::ofstream file(outputPath, std::ios::binary);
        if (!file.is_open()) {
            LM.writeLog("MeshCompiler - Failed to open output file: %s", outputPath.c_str());
            return false;
        }

        //calculate data size
        size_t dataSize = sizeof(CompiledMeshData) +
            meshData.positions.size() * sizeof(glm::vec3) +
            meshData.normals.size() * sizeof(glm::vec3) +
            meshData.colors.size() * sizeof(glm::vec3) +
            meshData.texCoords.size() * sizeof(glm::vec2) +
            meshData.indices.size() * sizeof(uint32_t);

        //write resource header
        CompiledResourceHeader resourceHeader{};
        resourceHeader.magic = CompiledResourceHeader::MAGIC_NUMBER;
        resourceHeader.version = CompiledResourceHeader::CURRENT_VERSION;
        resourceHeader.resourceType = static_cast<uint32_t>(ResourceType::MESH);
        resourceHeader.flags = 0;
        resourceHeader.dataSize = dataSize;
        resourceHeader.metadataSize = 0;
        resourceHeader.guid_instance = guid.m_Instance.m_Value;
        resourceHeader.guid_type = guid.m_Type.m_Value;
        resourceHeader.checksum = 0;
        resourceHeader.reserved = 0;

        //write to file
        file.write(reinterpret_cast<const char*>(&resourceHeader), sizeof(resourceHeader));

        //write mesh data header
        file.write(reinterpret_cast<const char*>(&header), sizeof(header));

        //write separate arrays
        file.write(reinterpret_cast<const char*>(meshData.positions.data()),
            meshData.positions.size() * sizeof(glm::vec3));

        file.write(reinterpret_cast<const char*>(meshData.normals.data()),
            meshData.normals.size() * sizeof(glm::vec3));

        file.write(reinterpret_cast<const char*>(meshData.colors.data()),
            meshData.colors.size() * sizeof(glm::vec3));

        file.write(reinterpret_cast<const char*>(meshData.texCoords.data()),
            meshData.texCoords.size() * sizeof(glm::vec2));

        file.write(reinterpret_cast<const char*>(meshData.indices.data()),
            meshData.indices.size() * sizeof(uint32_t));

        //check if all writes succeeded 
        if (!file) {
            LM.writeLog("MeshCompiler - Write error occurred");
            return false;
        }

        file.close();
        return true;
    }
} // namespace gam300