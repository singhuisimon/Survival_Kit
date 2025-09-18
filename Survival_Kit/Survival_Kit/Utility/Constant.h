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


namespace lof {

	//// ------------------------------ Serialization_Manager.cpp --------------------------------
	//// Screen Configuration Constants
	//constexpr int DEFAULT_SCREEN_WIDTH = 800;
	//constexpr int DEFAULT_SCREEN_HEIGHT = 600;

	//// FPS Display Constants
	//constexpr float DEFAULT_FPS_DISPLAY_INTERVAL = 1.0f;

	//// ----------------------------- FPS_Manager.cpp -------------------------------------------
	//// FPS_Manager Constants
	//constexpr int DEFAULT_TARGET_FPS = 120;
	//constexpr int64_t DEFAULT_MICROSECONDS_PER_SECOND = 1000000;
	//constexpr int64_t DEFAULT_TARGET_TIME = DEFAULT_MICROSECONDS_PER_SECOND / DEFAULT_TARGET_FPS;

	//constexpr int64_t DEFAULT_ADJUST_TIME = 0;
	//constexpr float DEFAULT_DELTA_TIME = 0.0f;
	//constexpr int64_t DEFAULT_LAST_FRAME_START_TIME = 0;

	//constexpr float DEFAULT_FIXED_DELTA_TIME = 1.0f / DEFAULT_TARGET_FPS;
	//constexpr float DEFAULT_ACCUMULATED_TIME = 0.0f;
	//constexpr int DEFAULT_NUMBER_OF_STEPS = 0;
	//constexpr int DEFAULT_MAX_STEPS = 3;

	//// ------------------------- System.cpp and Manager.cpp Files -------------------------------
	//// System Performance Constants
	//constexpr int64_t DEFAULT_START_TIME = 0;

	//// ----------------------------------- FPS.cpp ----------------------------------------------
	//// System Performance Constants
	//constexpr float UPON_PERCENTAGE = 100.0f;
	//constexpr int DEBUG_LEFT_WIDTH = 17;
	//constexpr int DEBUG_RIGHT_WIDTH = 5;

	//// ------------------------------ Matrix3x3.cpp --------------------------------------------
	//constexpr float PI_VALUE = 3.1415926f;
	//constexpr float PI_VALUE_DEGREES = 180.0f;


	//// -------------------------- Time Conversion Constants-----------------------------------
	//constexpr int MILLISECONDS_PER_SECOND = 1000;
	//constexpr int SECONDS_PER_MINUTE = 60;
	//constexpr int MINUTES_PER_HOUR = 60;
	//constexpr int HOURS_PER_DAY = 24;



} // namespace lof

#endif // CONSTANTS_H
