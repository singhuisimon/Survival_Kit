#pragma once
#ifndef __MESH_COMPILER_H__
#define __MESH_COMPILER_H__

#include "ResourceCompiler.h"

namespace gam300 {

    struct CompiledMeshData {
        uint32_t vertexCount;
        uint32_t indexCount;
        uint32_t reserved[2];
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
    };

} // namespace gam300

#endif // __MESH_COMPILER_H__