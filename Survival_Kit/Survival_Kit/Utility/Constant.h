/**
 * @file Constant.h
 * @brief Defines constant values used across the project.
 * @details Holds constants that can be utilize in the game engine.
 * @author 
 * @date 
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once
#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <string>

 // Include OpenGL headers
#define GLFW_INCLUDE_NONE
#include "../Glad/glad.h"
#include <GLFW/glfw3.h>
#include <glm-0.9.9.8/glm/glm.hpp>
#include <glm-0.9.9.8/glm/gtc/type_ptr.hpp>


namespace gam300 {

	//// ------------------------------ GraphicsManager.cpp --------------------------------
	constexpr int GPU_QUERY_COUNT = 3;



} // namespace lof

#endif // CONSTANTS_H
