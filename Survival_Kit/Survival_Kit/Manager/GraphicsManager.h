/**
 * @file GraphicsManager.h
 * @brief Declaration of the Graphics Manager for the game engine.
 * @details Manages graphical related operations in the game scene.
 * @author Chua Wen Bin Kenny
 * @date 10 September 2025
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
#include "../Graphics/Renderer.h"
#include "../Graphics/Material.h"
#include "../Graphics/Framebuffer.h" 
#include "../Graphics/Texture.h"


// For IMGUI operations
#include "ImguiManager.h"

// For Tracy operations
#include "../Tracy/tracy/Tracy.hpp"
#include "../Tracy/tracy/TracyOpenGL.hpp"

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

        // Query Handles
        GLuint gpuStartQueries[GPU_QUERY_COUNT]{};
        GLuint gpuEndQueries[GPU_QUERY_COUNT]{};
        int currentQueryIndex = 0;
        bool m_queriesCreated = false;

        Renderer m_renderer;

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

        /**
        * @brief Load all shader programs at start up.
        * @param shaders Storage for pairs of vertex and fragment shaders .
        * @return true if successful, false if unsuccessful.
        */
        bool loadShaderPrograms(std::vector<std::pair<std::string, std::string>> shaders);

        /**
        * @brief Return the framebuffer texture for ImGui.
        * @return texture handle of framebuffer texture.
        */
        GLuint getImguiTex() const { return m_renderer.get_imgui_texture(); }

        /**
        * @brief Return a reference to the Renderer object member.
        * @return referenced to Renderer object member.
        */
        Renderer& getRenderer() { return m_renderer; }

        /**
        * @brief Return a reference to the materials storage.
        * @return reference to materials storage.
        */
        const std::vector<Material>& getMaterialStorage() { return m_renderer.getMaterialStorage(); }

        /**
        * @brief Return a reference to the meshdata storage.
        * @return reference to meshdata storage.
        */
        const std::vector<MeshData>& getMeshDataStorage() { return m_renderer.getMeshDataStorage(); }

        /**
        * @brief Return a reference to the texture storage.
        * @return reference to texture storage.
        */
        const std::vector<Texture>& getTextureStorage() { return m_renderer.getTextureStorage(); }
        
        /**
        * @brief Return the size of the mesh count in the Renderer.
        * @return mesh count from the Renderer.
        */
        const size_t getMeshCount() const { return m_renderer.mesh_count(); }

        /**
        * @brief Return a Mesh name with the provide handle.
        * @return Mesh name in string form.
        */
        std::string getMeshName(uint16_t handle) const;

        /**
        * @brief Return a pointer to a Material object with the provide handle.
        * @return pointer to a Material object.
        */
        Material* getMaterial(uint16_t handle);

        /**
        * @brief Return a Mesh GUID with the provide handle.
        * @return Mesh GUID in string form.
        */
        std::string getMeshGUID(uint16_t handle) const;

        /**
        * @brief Handles the termination of GPU queries during abrupt shutdowns.
        */
        void preShutdownGPU();
    };

} // end of namespace gam300
#endif // __GRAPHICS_MANAGER_H__