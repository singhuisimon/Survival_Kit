#pragma once
/**
 * @file ImguiManager.h
 * @brief Declaration of the IMGUI_Manager class for running the IMGUI level editor.
 * @author Liliana Hanawardani (45%), Saw Hui Shan (45%), Rio Shannon Yvon Leonardo (10%)
 * @date September 8, 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#ifndef SK_IMGUI_MANAGER_H
#define SK_IMGUI_MANAGER_H

// Include header file
#include "Manager.h"

// Include Imgui Header files
#include "../IMGUI/imgui.h"
#include "../IMGUI/imgui_impl_glfw.h"
#include "../IMGUI/imgui_impl_opengl3.h"

// Include Standard Headers
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>

// Include other necessary headers
#include "../Component/Transform3D.h"
#include "../Utility/Vector3D.h"
#include "../Manager/GraphicsManager.h"

// Include Asset pipeline headers
#include "../Pipeline/AssetDatabase.h"
#include "../Pipeline/DescriptorEditor.h"
#include "AssetManager.h"

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

		// Reference to the existing ECS Manger to access entities
		class ECSManager& ImguiEcsRef;

		// Reference to the existing Graphics Manger for Viewport
		class GraphicsManager& ImguiGraphicRef;

		// Variables to store window dimensions
		int width = 0;
		int height = 0;

		// IMGUI (Open Scene, Save As, Asset, Prefab, Script) Windows functionality
		bool fileWindow = false;
		std::string shownFile{};
		char saveAsDefaultName[128] = {};
		bool showSaveAsPanel = false;
		int selectedAssetIndex = -1;
		bool showPrefabPanel = false;
		bool showScriptOptions = false;
		bool makeScript = false;
		bool showReplacePrefab = false;

		// Asset Browser functionality
		void refreshAssetList();
		const char* getAssetTypeName(AssetType type);
		void initializeAssetBrowser();

		// Descriptor Editor functionality
		DescriptorEditor* m_descriptorEditor;
		EditableDescriptor m_currentDescriptor;
		bool m_showDescriptorPanel = false;

		// Performance profile 
		float lastReceivedFPS = 0.0f;
		static const int FPS_HISTORY_SIZE = 90;  //store up to 90 frames
		float fpsHistory[FPS_HISTORY_SIZE] = {};
		float frameTimeHistory[FPS_HISTORY_SIZE] = {}; //for frame time
		int fpsHistoryOffset = 0;

		// Statistics tracking
		float minFPS = FLT_MAX;
		float maxFPS = 0.0f;
		float minFrameTime = FLT_MAX;
		float maxFrameTime = 0.0f;


	public:

		/**************************************************************************
		* @brief Constructs an ImguiManager instance with references to ECS and Graphics managers.
		* @param ecsManager Reference to the ECSManager.
		* @param GFM Reference to the GraphicsManager.
		**************************************************************************/
		ImguiManager(ECSManager& ecsManager, GraphicsManager& GFM);

		/**************************************************************************
		* @brief Deleted copy constructor. 
		**************************************************************************/
		ImguiManager(const ImguiManager&) = delete;

		/**************************************************************************
		* @brief Deleted copy assignment operator to prevent assignment.
		**************************************************************************/
		ImguiManager& operator=(const ImguiManager&) = delete;

		/**************************************************************************
		* @brief Returns the singleton instance of ImguiManager.
		* @return Reference to the ImguiManager instance.
		**************************************************************************/
		static ImguiManager& getInstance();

		/**************************************************************************
		* @brief Initializes the ImGui. 
		* @return Throws errors on failure
		**************************************************************************/
		int startUp() override;

		/**************************************************************************
		* @brief Initializes ImGui with a specific GLFW window and IO context.
		* @param glfwindow Pointer reference to the GLFW window.
		* @param imgui_io Reference to the ImGuiIO structure for input/output handling.
		* @return Returns 0 on success or an error code on failure.
		**************************************************************************/
		int startUp(GLFWwindow*& glfwindow, ImGuiIO& imgui_io);

		/**************************************************************************
		* @brief Start imgui frame
		**************************************************************************/
		void startImguiFrame();

		/**************************************************************************
		* @brief Renders the viewport window showing the scene view.
		**************************************************************************/
		void renderViewport();

		//void displayTopMenuBar();
		/**************************************************************************
		* @brief Displays the list of files 
		**************************************************************************/
		void displayFileList();
	
		/**************************************************************************
		* @brief Displays the hierarchy window containing all entities in the scene.
		**************************************************************************/
		void displayHierarchyList();

		/**************************************************************************
		* @brief Displays the properties panel showing components of the selected entity.
		**************************************************************************/
		void displayPropertiesList();

		/**************************************************************************
		* @brief Displays the asset browser to browse the available asset
		**************************************************************************/
		void displayAssetsBrowserList();	//asset browser
	
		/**************************************************************************
		* @brief Displays the top menu bar
		**************************************************************************/
		void displayTopMenu();

		/*void showPrefabsPanel(EntityID selectedEntity);*/
		/**************************************************************************
		* @brief Finalizes and renders the ImGui frame to the screen.
		* @param imgui_io Reference to the ImGuiIO structure for rendering input state.
		**************************************************************************/
		void finishImguiRender(ImGuiIO& imgui_io);

		/**************************************************************************
		* @brief Handles mouse click events in the viewport
		* @param mousePos The mouse position in ImGui coordinates.
		* @param viewportSize The size of the viewport
		**************************************************************************/
		void handleViewPortClick(ImVec2 mousePos, ImVec2 viewportSize);

		/**************************************************************************
		* @brief Displays the asset editor interface for editing asset data.
		**************************************************************************/
		void displayAssetEditor();

		/**************************************************************************
		* @brief Displays the prefab editor
		* @param prefabFilepath Filesystem entry pointing to the prefab file.
		**************************************************************************/
		void displayPrefabEditor(const std::filesystem::directory_entry& prefabFilepath);

		/**************************************************************************
		* @brief Shuts down and cleans up ImGui resources.
		**************************************************************************/
		void shutDown() override;

		/**************************************************************************
		* @brief Retrieves the window size from a GLFW window.
		* @param window Reference to the GLFW window.
		* @return 2D vector containing width and height of the window.
		**************************************************************************/
		Vector2D getWindowSize(GLFWwindow& window);

		/**************************************************************************
		* @brief Returns the stored window width and height for ImGui textures/FBO.
		* @return Vector2D of width and height
		**************************************************************************/
		Vector2D getWindowWidthHeight() { return Vector2D(static_cast<float>(width), static_cast<float>(height)); }

		/**************************************************************************
		* @brief Displays the performance profiling panel
		**************************************************************************/
		void displayPerformanceProfile();

		/**************************************************************************
		* @brief template to add the remove component menu right beside collapsing menu
		* @tparam componentType The type of the component to display.
		* @param entityID The ID of the entity 
		* @param componentName The name of the component
		**************************************************************************/
		// template to add the remove component menu right beside collapsing menu
		template<typename componentType>
		void displayComponentMenu(EntityID entityID, const char* componentName);

		/**************************************************************************
		* @brief Displays the content of the component
		* @tparam componentType The type of the component.
		* @param selectedEntityID The ID of the selected entity.
		**************************************************************************/
		template<typename componentType>
		void displayComponentContent(EntityID selectedEntityID);

		/**************************************************************************
		* @brief Initializes the asset manager used by the asset browser.
		**************************************************************************/
		void initializeAssetManager();

		/**************************************************************************
		* @brief Rescans asset directories to update the asset browser list.
		**************************************************************************/
		void rescanAssets();

		/**************************************************************************
		* @brief Updates the FPS counter for performance profiling display.
		* @param fps Frames per second value.
		**************************************************************************/
		void updateFPS(float fps);

	};


} // end of namespace gam300
#endif // LOF_IMGUI_MANAGER_h