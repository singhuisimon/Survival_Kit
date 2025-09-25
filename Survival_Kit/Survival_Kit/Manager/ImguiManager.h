#pragma once
/**
 * @file ImguiManager.h
 * @brief Declaration of the IMGUI_Manager class for running the IMGUI level editor.
 * @author Liliana Hanawardani, Saw Hui Shan
 * @date September 8, 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#ifndef SK_IMGUI_MANAGER_H
#define SK_IMGUI_MANAGER_H

// Include header file
#include "Manager.h"

// Include Imgui Header file
#include "../IMGUI/imgui.h"
#include "../IMGUI/imgui_impl_glfw.h"
#include "../IMGUI/imgui_impl_opengl3.h"

// Include Standard Headers
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>
//#include <GLFW/glfw3.h>

// Include other necessary headers
#include "../Component/Transform3D.h"
#include "../Utility/Vector3D.h"
#include "../Manager/GraphicsManager.h"


#define IMGUIM gam300::ImguiManager::getInstance()

namespace gam300
{

	/**
	* @class ImguiManager
	* @brief Manages the starting up, shutting down, the displaying and editing
	* information, and the internal logic in level editor.
	*
	* This class follows the Singleton pattern to ensure a single instance throughout
	* the application's lifecycle.
	*/
	class ImguiManager : public Manager {
	private:

		/**
		 * @brief Private constructor for singleton pattern
		 */
		ImguiManager();

		// reference to the existing ECS Manger to access entitties
		class ECSManager& ImguiEcsRef;
		class GraphicsManager& ImguiGraphicRef;
		int width = 0;
		int height = 0;
		
	
	public:

		ImguiManager(ECSManager& ecsManager, GraphicsManager& GFM);

		ImguiManager(const ImguiManager&) = delete;

		ImguiManager& operator=(const ImguiManager&) = delete;

		static ImguiManager& getInstance();

		int startUp() override;

		int startUp(GLFWwindow*& glfwindow, ImGuiIO& imgui_io);

		void startImguiFrame();

		void renderViewport();

		//void displayTopMenuBar();

		void displayFileList(bool& fileWindow, std::string& shownFile);
		//void displayFileList();

		void displayHierarchyList();

		void displayPropertiesList();

		void displayAssetsBrowserList();

		void finishImguiRender(ImGuiIO& imgui_io);

		void shutDown() override;

		// to get the window size from main.cpp
		Vector2D getWindowSize(GLFWwindow& window);
		
		// to retuen the width and height for imguiTex and imguiFbo
		Vector2D getWindowWidthHeight() { return Vector2D(width, height); }

		// template to add the remove component menu right beside collapsing menu
		template<typename componentType>
		void displayComponentMenu(EntityID entityID, const char* componentName);

		template<typename componentType>
		void displayComponentContent(EntityID selectedEntityID);

	};


} // end of namespace gam300
#endif // LOF_IMGUI_MANAGER_h