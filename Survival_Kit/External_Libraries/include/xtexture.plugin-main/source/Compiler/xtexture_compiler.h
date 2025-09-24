#ifndef TEXTURE_COMPILER_H
#define TEXTURE_COMPILER_H
#pragma once

#include "dependencies/xresource_pipeline_v2/source/xresource_pipeline.h"

namespace xtexture_compiler
{
    enum class state : std::uint32_t
    { OK
    , FAILURE
    };

    struct instance : xresource_pipeline::compiler::base
    {
        static std::unique_ptr<instance> Create(void);
    };
}

#endif
