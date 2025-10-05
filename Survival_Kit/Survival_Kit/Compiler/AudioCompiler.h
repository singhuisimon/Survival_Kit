/**
 * @file AudioCompiler.h
 * @brief Audio resource compiler.
 * @details Compiles intermediate audio files to optimized binary format.
 * @author Simon Chan
 * @date October 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once
#ifndef __AUDIO_COMPILER_H__
#define __AUDIO_COMPILER_H__

#include "ResourceCompiler.h"

namespace gam300 {

    class AudioCompiler : public IResourceCompiler {
    public:
        CompileResult compile(
            const std::string& intermediatePath,
            const ResourceProperties* properties,
            ResourcePaths& paths,
            const xresource::full_guid& guid
        ) override;

        ResourceType getResourceType() const override {
            return ResourceType::AUDIO;
        }
    };

} // namespace gam300

#endif // __AUDIO_COMPILER_H__