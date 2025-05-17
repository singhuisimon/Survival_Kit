/**
 * @file RenderFactory.h
 * @brief Declaration of a factory method for creating renderers.
 * @details This file defines the interface for creating a renderer based
 * on a specified graphics API. It uses the factory method to abstract the
 * creation of API-specific renderer implementations (e.g. OpenGL, Vulkan).
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once
#ifndef __RENDER_FACTORY_H__
#define __RENDER_FACTORY_H__

#include <memory>
#include "IRenderer.h"

namespace gam300 {
	std::unique_ptr<IRenderer> MakeRenderer(GFXAPI api);
}

#endif