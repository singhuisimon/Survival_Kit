#pragma once
#ifndef __SHADER_COMPILER_H__
#define __SHADER_COMPILER_H__

#include "ResourceCompiler.h"

namespace gam300 {

    class ShaderCompiler : public IResourceCompiler {
    public:
        CompileResult compile(
            const std::string& intermediatePath,
            const ResourceProperties* properties,
            ResourcePaths& paths,
            const xresource::full_guid& guid
        ) override;

        ResourceType getResourceType() const override {
            return ResourceType::SHADER;
        }
    };

} // namespace gam300