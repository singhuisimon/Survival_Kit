/**
 * @file RenderFactory.cpp
 * @brief Implementation of a factory method for creating renderers.
 * @details This file contains the implementation of a factory method for 
 * creating a renderer based on a specified graphics API.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#include <stdexcept>
#include "RenderFactory.h"
#include "GLRenderer.h"

namespace gam300 {

	std::unique_ptr<IRenderer> MakeRenderer(GFXAPI api) {

		switch (api) 
		{
			case GFXAPI::OpenGL: return std::make_unique<GLRenderer>();
			case GFXAPI::Vulkan: throw std::runtime_error("Vulkan is currently unsupported.");
			default: throw std::runtime_error("Unsupported Graphics API.");
		}
	}
}