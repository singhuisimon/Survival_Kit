/**
 * @file Main.h
 * @brief Main header file for the game engine application.
 * @details Includes necessary libraries and headers for the application entry point.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once
#ifndef __MAIN_H__
#define __MAIN_H__

// Include thread/chrono
#include <thread>
#include <chrono>

// Include Manager headers using consistent paths
#include "../Manager/Manager.h"
#include "../Manager/GameManager.h"
#include "../Manager/LogManager.h"
#include "../Manager/InputManager.h"
#include "../Manager/ECSManager.h"
#include "../Manager/ImguiManager.h"
#include "../Utility/Clock.h"
#include "../Manager/GraphicsManager.h" // GLFW & GLAD headers

// Include IMGUI headers
#include "../IMGUI/imgui.h"
#include "../IMGUI/imgui_impl_glfw.h"
#include "../IMGUI/imgui_impl_opengl3.h"


#include "Application.h"
#endif // __MAIN_H__