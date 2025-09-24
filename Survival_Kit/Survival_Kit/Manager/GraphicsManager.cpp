/**
 * @file GraphicsManager.cpp
 * @brief Implementation of the Graphics Manager for the game engine.
 * @details Handles graphical related operations in the game scene.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#include "GraphicsManager.h"

#include <glm-0.9.9.8/glm/glm.hpp>
#include <glm-0.9.9.8/glm/gtc/quaternion.hpp>
#include <glm-0.9.9.8/glm/gtx/quaternion.hpp>
#include "../Component/Transform3D.h"

namespace gam300 {

    // Initialize singleton instance
    GraphicsManager::GraphicsManager() {
        setType("GraphicsManager");
    }

    // Get the singleton instance
    GraphicsManager& GraphicsManager::getInstance() {
        static GraphicsManager instance;
        return instance;
    }

    // Start up the GraphicsManager
    int GraphicsManager::startUp() {
        // Call parent's startUp() first
        if (Manager::startUp())
            return -1;

        // Load OpenGL function pointers with GLAD
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            LM.writeLog("GraphicsManager::startUp(): Failed to initialize OpenGL function pointers!");
            std::cerr << "GraphicsManager::startUp(): Failed to initialize OpenGL function pointers!" << std::endl;
            return -1;
        }
        else {
            LM.writeLog("GraphicsManager::startUp(): GLAD initialized successfully.");
        }

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        //glViewport(0, 0, IMGUIM.getWindowWidthHeight().x, IMGUIM.getWindowWidthHeight().y);

        //// Set framebuffer with color (Background color)
        //glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        //glViewport(0, 0, WC.get_win_width(), WC.get_win_height());

        //// Set up default render mode 
        //render_mode = GL_FILL;
        //std::string vertex_obj_path = ASM.get_full_path(ASM.SHADER_PATH, "lack_of_oxygen_obj.vert");
        //std::string fragment_obj_path = ASM.get_full_path(ASM.SHADER_PATH, "lack_of_oxygen_obj.frag");
        //std::string vertex_debug_path = ASM.get_full_path(ASM.SHADER_PATH, "lack_of_oxygen_debug.vert");
        //std::string fragment_debug_path = ASM.get_full_path(ASM.SHADER_PATH, "lack_of_oxygen_debug.frag");
        //std::string vertex_font_path = ASM.get_full_path(ASM.SHADER_PATH, "lack_of_oxygen_font.vert");
        //std::string fragment_font_path = ASM.get_full_path(ASM.SHADER_PATH, "lack_of_oxygen_font.frag");

        // Temporary hardcoded filepath for vertex and fragment shaders
        //std::string vertex_obj_path{ "..\\..\\Assets\\Shaders\\survival_kit_obj.vert" };
        //std::string fragment_obj_path{ "..\\..\\Assets\\Shaders\\survival_kit_obj.frag" };
        std::string vertex_obj_path{ "..\\Survival_Kit\\Assets\\Shaders\\survival_kit_obj.vert" };
        std::string fragment_obj_path{ "..\\Survival_Kit\\Assets\\Shaders\\survival_kit_obj.frag" };

        // Pair vertex and fragment shader files
        std::vector<std::pair<std::string, std::string>> shader_files{
            std::make_pair(vertex_obj_path, fragment_obj_path),
        };

        // Load shader files
        if (!loadShaderPrograms(shader_files)) {
            LM.writeLog("GraphicsManager::startUp(): Failed to load shader programs");
            std::cerr << "GraphicsManager::startUp(): Failed to load shader programs" << std::endl;
            return -1;
        } else {
            LM.writeLog("GraphicsManager::startUp(): Succesfully added shader programs.");
        }

        // Set camera as orbiting
        main_camera = Camera3D(ORBITING, glm::vec3(0.0f, 5.0f, 5.0f), glm::vec3(0.f, 0.f, 0.0f), 45.0f, 0.5f, 100.0f);

        //// File path for assets
        //std::string mesh_path = ASM.get_full_path(ASM.MODEL_PATH, DEFAULT_MODEL_MSH_FILE);
        //std::string animation_path = ASM.get_full_path(ASM.TEXTURE_PATH, DEFAULT_ATLAS_FILE);
        //std::string font_path = ASM.get_full_path(ASM.FONT_PATH, DEFAULT_FONTS_FILE);

        // Add models

        // Add animations

        // Add fonts

        // Creating framebuffer object for IMGUI viewport
        auto temp_fbo = FrameBuffer::create();
        if (temp_fbo->valid()) {
            imgui_fbo = std::move(temp_fbo);
        }

        // Set up the framebuffer and game scene texture for imgui viewport
        glBindFramebuffer(GL_FRAMEBUFFER, imgui_fbo->handle());
        
        // Creating texture object for imgui
        int windowWidth = 640;
        int windowHeight = 480;
        //int windowWidth = IMGUIM.getWindowWidthHeight().x;
        //int windowHeight = IMGUIM.getWindowWidthHeight().y;
        glGenTextures(1, &imguiTex);
        glBindTexture(GL_TEXTURE_2D, imguiTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth , windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // Attaching texture object for imgui to framebuffer 
        imgui_fbo->attach_color(GL_COLOR_ATTACHMENT0, imguiTex);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);

        MeshData cubeData = Shape::make_cube();
        MeshData planeData = Shape::make_plane();
        MeshData sphereData = Shape::make_sphere();

        MeshGL   cubeGL = Shape::upload_mesh_data(cubeData);
        MeshGL   planeGL = Shape::upload_mesh_data(planeData);
        MeshGL   sphereGL = Shape::upload_mesh_data(sphereData);

        //meshStorage.push_back(std::move(cubeGL));
        //meshStorage.push_back(std::move(planeGL));
        meshStorage.push_back(std::move(sphereGL));


        // Log startup
        LM.writeLog("GraphicsManager::startUp() - Graphics Manager started successfully");
        return 0;
    }

    // Shut down the GraphicsManager
    void GraphicsManager::shutDown() {
        // Log shutdown
        LM.writeLog("GraphicsManager::shutDown() - Shutting down Graphics Manager");

        // Reset/Clear anything if needed

        //// Clear stored states
        //m_key_states.clear();
        //m_prev_key_states.clear();

        // Call parent's shutDown()
        Manager::shutDown();
    }

    // Update input states, should be called once per frame
    void GraphicsManager::update() {

        // update loop 
        /*
        to include:
        -
        -
        -
        -
        -
        */

        auto x = glm::angleAxis(glm::radians(0.0f), glm::vec3(0, 0, 1));
        auto y = glm::angleAxis(glm::radians(0.0f), glm::vec3(0, 1, 0));

        auto xy = x * y;

        // Calculate the model to world transform
        glm::mat4 scale_matrix = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
        glm::mat4 rot_matrix = glm::toMat4(xy);
        glm::mat4 trans_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.f, 0.f));

        glm::mat4 TRS = trans_matrix * rot_matrix * scale_matrix;

        Transform3D transform;

        transform.setPosition(Vector3D(0.0f, 0.0f, 0.0f));
        transform.setScale(Vector3D(1.0f, 1.0f, 1.0f)); // Don't set any values to 0
        transform.setRotation(Vector3D(0.0f, 0.0f, 0.0f));

        shadersStorage[0].programUse();

        // Temporary transformations for camera
        shadersStorage[0].setUniform("M", transform.getTransformationMatrix()); // Model transform
        shadersStorage[0].setUniform("V", main_camera.getLookAt()); // View transform
        shadersStorage[0].setUniform("P", main_camera.getPerspective()); // Perspective transform

        // Temporary input for cursor to move camera
        if (IM.isKeyPressed(GLFW_KEY_LEFT_SHIFT)) {
            std::cout << IM.getMouseDeltaX() << std::endl;
            main_camera.cameraOnCursor(IM.getMouseDeltaX(), IM.getMouseDeltaY(), &shadersStorage[0]);
        }

        //// KENNY TESTING: Temporary input for scroll
        //if (IM.isKeyPressed(GLFW_KEY_RIGHT_SHIFT)) {
        //    std::cout << IM.getScrollY() << std::endl;
        //    main_camera.cameraOnScroll(IM.getScrollY(), &shadersStorage[0]);
        //}

        // Set uniform to shader after update light values
        shadersStorage[0].setUniform("light.position", main_light.getLightPos());  // Position
        shadersStorage[0].setUniform("light.La", main_light.getLightAmbient());        // Ambient
        shadersStorage[0].setUniform("light.Ld", main_light.getLightDiffuse());        // Diffuse
        shadersStorage[0].setUniform("light.Ls", main_light.getLightSpecular());        // Specular

        //Temporary input for light cursor
        if (IM.isKeyPressed(GLFW_KEY_L)) {
            //std::cout << IM.getMouseDeltaX() << std::endl;
            main_light.lightOnCursor(IM.getMouseDeltaX(), IM.getMouseDeltaY(), &shadersStorage[0]);
        }

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS); // Default comparison

        // Bind framebuffer object for IMGUI viewport
        glBindFramebuffer(GL_FRAMEBUFFER, imgui_fbo->handle());

        // Clear the color and depth buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // temporary comment for the imgui

        for (auto const& mesh : meshStorage) {

            mesh.vao.bind();

            // Draw the actual object
            glDrawElements(mesh.primitive_type, mesh.draw_count, mesh.index_type, NULL);

            glBindVertexArray(0);
        }

        shadersStorage[0].programFree();

        // Unbind framebuffer object
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    bool GraphicsManager::loadShaderPrograms(std::vector<std::pair<std::string, std::string>> shaders) {
        for (auto const& file : shaders) { 
            // Create the shader files vector with types 
            std::vector<std::pair<GLenum, std::string>> shader_files; 
            shader_files.emplace_back(std::make_pair(GL_VERTEX_SHADER, file.first)); 
            shader_files.emplace_back(std::make_pair(GL_FRAGMENT_SHADER, file.second)); 

            // Create new shader program
            ShaderProgram shader_program; 

            // Use Graphics_Manager to compile the shader
            if (!shader_program.compileShader(shader_files)) {
                LM.writeLog("GraphicsManager::loadShaderPrograms(): Shader program failed to compile.");
                return false;
            }

            // Insert shader program into vector
            shadersStorage.emplace_back(shader_program);
            std::size_t shader_idx = shadersStorage.size() - 1;

            LM.writeLog("GraphicsManager::loadShaderPrograms(): Shader program handle is %u.", shader_program.getShaderProgramHandle());
            LM.writeLog("GraphicsManager::loadShaderPrograms(): Shader program %zu created, compiled and added successfully.", shader_idx);
        }
        return true;
    }


} // end of namespace gam300