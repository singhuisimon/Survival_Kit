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

// For accessing entities
#include "ECSManager.h"

// To support graphical operations
#include "../Utility/Constant.h"
#include "../Graphics/ShaderProgram.h"
#include "../Graphics/Camera.h"
#include "../Graphics/Light.h"
#include "../Graphics/Shape.h"
#include "../Graphics/Material.h"
#include "../Graphics/Framebuffer.h" 
#include "../Graphics/Texture.h"


// For IMGUI operations
#include "ImguiManager.h"

// KENNY TESTING: For testing cursor input
#include "InputManager.h"

// For creating asset path
#include "../Utility/AssetPath.h"

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
        std::vector<MeshData>      m_meshDataStorage;
        std::vector<std::optional<Texture>>       m_textureStorage;

        // Storage for materials
        std::map<uint16_t, Material> m_material_storage;
        
        // Main camera
        Camera3D main_camera;

        // Main light
        Light main_light;

        // Framebuffer object and texture for IMGUI
        GLuint imguiTex{ 0 };
        std::optional<FrameBuffer> imgui_fbo; 

        // Mesh selection
        int selected_texture{ 0 };

        // Texture flag
        bool textureMode = false;

        // Render type
        bool isPBR = false;

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

        GLuint getImguiTex() { return imguiTex; }
        //GLuint getImguiFbo() { return imguiFbo; }

        // Get materials storage
        const std::map<uint16_t, Material>& getMaterialStorage() { return m_material_storage; }

        // Get meshdata storage
        const std::vector<MeshData>& getMeshDataStorage() { return m_meshDataStorage; }

        // Get texture storage
        const std::vector<std::optional<Texture>>& getTextureStorage() { return m_textureStorage; }
        
        size_t getMeshCount() const { return meshStorage.size(); }


        std::string getMeshName(uint16_t handle) const;
        Material* getMaterial(uint16_t handle);

        std::string getMeshGUID(uint16_t handle) const;
    };

} // end of namespace gam300
#endif // __GRAPHICS_MANAGER_H__