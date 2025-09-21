/**
 * @file GraphicsManager.h
 * @brief Declaration of the Graphics Manager for the game engine.
 * @details Manages graphical related operations in the game scene.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once
#ifndef __GRAPHICS_MANAGER_H__
#define __GRAPHICS_MANAGER_H__

#include "Manager.h"
#include <unordered_map>
#include <array>
#include <vector>

// For logging/debugging information
#include <iostream>
#include "LogManager.h"

// To support graphical operations
#include "../Utility/Constant.h"
#include "../Graphics/ShaderProgram.h"
#include "../Graphics/Camera.h"
#include "../Graphics/Light.h"
#include "../Graphics/Shape.h"

// KENNY TESTING: For testing cursor input
#include "InputManager.h"


// Things to include or KIV for creating a scene
/* 
- shader program (P)
- fbo (JR)
- triangle meshes (JR)
- camera (P)
- light 
- material
- texture (JR)
- fonts
- animations
- particles 
*/

 // Two-letter acronym for easier access to manager.
#define GFXM gam300::GraphicsManager::getInstance()

namespace gam300 {

    class GraphicsManager : public Manager {
    private:
        GraphicsManager();                      // Private since a singleton.

        // Storage for shader programs (Will port to asset manager eventually)
        std::vector<ShaderProgram> shadersStorage;
        std::vector<MeshGL>        meshStorage;
        
        // Main camera
        Camera3D main_camera;

        // Main light
        Light main_light;

    public:
        /**
         * @brief Get the singleton instance of the GraphicsManager.
         * @return Reference to the singleton instance.
         */
        static GraphicsManager& getInstance();

        /**
         * @brief Start up the GraphicsManager.
         * @return 0 if successful, else -1.
         */
        int startUp() override;

        /**
         * @brief Shut down the GraphicsManager.
         */
        void shutDown() override;

        /**
         * @brief Update input states, should be called once per frame.
         */
        void update();

        // To load all shader program at start up (the pair of 2 strings are the vertex and fragment shaders' filepath)
        bool loadShaderPrograms(std::vector<std::pair<std::string, std::string>> shaders);

    };

} // end of namespace gam300
#endif // __GRAPHICS_MANAGER_H__