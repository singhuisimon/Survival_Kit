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
#include "../Component/MeshComponent.h"

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
        
        //glEnable(GL_DEPTH_TEST);
        //glDepthFunc(GL_LESS); // Default comparison
        //glClearDepth(1.0f);
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

        // Filepath for vertex and fragment shaders
        std::string vertex_obj_path{ getAssetFilePath("Shaders/survival_kit_obj.vert") };
        std::string fragment_obj_path{ getAssetFilePath("Shaders/survival_kit_obj.frag") };

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

        // Temporarily load textures 
        auto mouse_tex = Texture::load_from_file(getAssetFilePath("Textures/mouse_kenny.png"), TextureDesc(false, false, true));
        if (mouse_tex->valid()) {
            std::cout << "Mouse tex handle is " << mouse_tex->handle() << std::endl;
            m_textureStorage.push_back(std::move(mouse_tex));
        }

        auto rabbit_tex = Texture::load_from_file(getAssetFilePath("Textures/rabbit_kenny.png"), TextureDesc(false, false, true));
        if (rabbit_tex->valid()) {
            std::cout << "Rabbit tex handle is " << rabbit_tex->handle() << std::endl;
            m_textureStorage.push_back(std::move(rabbit_tex));
        }

        auto squirrel_tex = Texture::load_from_file(getAssetFilePath("Textures/squirrel_kenny.png"), TextureDesc(false, false, true));
        if (squirrel_tex->valid()) { 
            std::cout << "Squirrel tex handle is " << squirrel_tex->handle() << std::endl; 
            m_textureStorage.push_back(std::move(squirrel_tex));
        }

        // Set camera as orbiting
        main_camera = Camera3D(ORBITING, glm::vec3(0.0f, 5.0f, 10.0f), glm::vec3(0.f, 0.f, 0.0f), 45.0f, 0.5f, 100.0f);

        // Set light
        main_light = Light(glm::vec3(0.0f, 5.0f, 0.0f),
            glm::vec3(0.4f, 0.4f, 0.4f),
            glm::vec3(1.0f, 1.0f, 1.0f),
            glm::vec3(1.0f, 1.0f, 1.0f));

        // Temporarily load materials manually here
        Material mat1 = Material(glm::vec3(0.3f, 0.5f, 0.9f), glm::vec3(0.3f, 0.5f, 0.9f), glm::vec3(0.8f, 0.8f, 0.8f), 100.0f);
        Material mat2 = Material(glm::vec3(0.9f, 0.5f, 0.3f), glm::vec3(0.9f, 0.5f, 0.3f), glm::vec3(0.8f, 0.8f, 0.8f), 100.0f);
        m_material_storage.emplace(m_material_storage.size(), mat1);
        m_material_storage.emplace(m_material_storage.size(), mat2);

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

        // Attaching renderbuffer 
        GLuint rboDepth; 
        glCreateRenderbuffers(1, &rboDepth);
        glNamedRenderbufferStorage(rboDepth, GL_DEPTH_COMPONENT24, windowWidth, windowHeight);
        imgui_fbo->attach_renderbuffer(GL_DEPTH_ATTACHMENT, rboDepth);

        // Unbind fbo and texture
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);

        MeshData cubeData = Shape::make_cube();
        MeshData planeData = Shape::make_plane();
        MeshData sphereData = Shape::make_sphere();

        // Save mesh data for collision calculation
        m_meshDataStorage.emplace_back(cubeData);
        m_meshDataStorage.emplace_back(planeData);
        m_meshDataStorage.emplace_back(sphereData);

        MeshGL   cubeGL = Shape::upload_mesh_data(cubeData);
        MeshGL   planeGL = Shape::upload_mesh_data(planeData);
        MeshGL   sphereGL = Shape::upload_mesh_data(sphereData);

        meshStorage.push_back(std::move(cubeGL));
        meshStorage.push_back(std::move(planeGL));
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

        // Temporary input for cursor to move camera
        if (IM.isKeyPressed(GLFW_KEY_LEFT_SHIFT)) {

            // MOUSE: Camera control with left mouse button held down
            if (IM.isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
                double mouseDeltaX = IM.getMouseDeltaX();
                double mouseDeltaY = IM.getMouseDeltaY();

                if (std::abs(mouseDeltaX) > 0.1 || std::abs(mouseDeltaY) > 0.1) {
                    // INCREASE SENSITIVITY
                    float extraSensitivity = 500.0f;

                    main_camera.cameraOnCursor(mouseDeltaX * extraSensitivity,
                        mouseDeltaY * extraSensitivity,
                        &shadersStorage[0]);
                }
            }
        }


        // KEYBOARD: Camera control with arrow keys
        float keyboardSensitivity = 8.0f; // Adjust this value for keyboard speed
        bool keyPressed = false;
        double keyDeltaX = 0.0;
        double keyDeltaY = 0.0;

        if (IM.isKeyPressed(GLFW_KEY_LEFT)) {
            keyDeltaX = -keyboardSensitivity;
            keyPressed = true;
        }
        if (IM.isKeyPressed(GLFW_KEY_RIGHT)) {
            keyDeltaX = keyboardSensitivity;
            keyPressed = true;
        }
        if (IM.isKeyPressed(GLFW_KEY_UP)) {
            keyDeltaY = -keyboardSensitivity;
            keyPressed = true;
        }
        if (IM.isKeyPressed(GLFW_KEY_DOWN)) {
            keyDeltaY = keyboardSensitivity;
            keyPressed = true;
        }

        // Handle diagonal movement (multiple keys pressed)
        if (keyPressed) {
            main_camera.cameraOnCursor(keyDeltaX, keyDeltaY, &shadersStorage[0]);
        }

        // Set up shader program
        shadersStorage[0].programUse();

        // Temporary transformations for camera
        //shadersStorage[0].setUniform("M", transform.getTransformationMatrix()); // Model transform
        shadersStorage[0].setUniform("V", main_camera.getLookAt()); // View transform
        shadersStorage[0].setUniform("P", main_camera.getPerspective()); // Perspective transform

        // Set uniform to shader after update light values
        shadersStorage[0].setUniform("light.position", main_light.getLightPos());       // Position
        shadersStorage[0].setUniform("light.La", main_light.getLightAmbient());         // Ambient
        shadersStorage[0].setUniform("light.Ld", main_light.getLightDiffuse());         // Diffuse
        shadersStorage[0].setUniform("light.Ls", main_light.getLightSpecular());        // Specular

        //Temporary input for light cursor
        if (IM.isKeyPressed(GLFW_KEY_L)) {
            //std::cout << IM.getMouseDeltaX() << std::endl;
            main_light.lightOnCursor(IM.getMouseDeltaX(), IM.getMouseDeltaY(), &shadersStorage[0]);
        }

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS); // Default comparison
        glClearDepth(1.0f);
        

        // Bind framebuffer object for IMGUI viewport
        glBindFramebuffer(GL_FRAMEBUFFER, imgui_fbo->handle());

        // Clear the color and depth buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

        // Enable choosing of mesh
        if (IM.isKeyPressed(GLFW_KEY_1)) {
            selected_texture = 0;
        }
        if (IM.isKeyPressed(GLFW_KEY_2)) {
            selected_texture = 1;
        }
        if (IM.isKeyPressed(GLFW_KEY_3)) {
            selected_texture = 2;
        }

        // Testing texture
        if (IM.isKeyPressed(GLFW_KEY_9)) {
            textureMode = false;
        } 
        if (IM.isKeyPressed(GLFW_KEY_0)) {
            textureMode = true;
        }
        if (textureMode) {
            glBindTextureUnit(0, m_textureStorage[selected_texture]->handle());
            shadersStorage[0].setUniform("Texture2D", 0);
            shadersStorage[0].setUniform("isTexture", true);
        }
        else {
            shadersStorage[0].setUniform("isTexture", false);
        }

        // Default mesh and material handle
        uint16_t mesh_handle = 0;
        uint16_t material_handle = 0;

        // KENNY TESTING: ACCESSING ENTITIES AND UPDATING THEIR TRANSFORMS PER FRAME     
        const auto transform_entities_IDs = EM.getEntitiesWithComponent<Transform3D>();
        for (const auto entity_id : transform_entities_IDs) {

            // Get entity's transform component using ID
            if (EM.hasComponent<Transform3D>(entity_id)) { // Extra check just in case 
                Transform3D* transform = EM.getComponent<Transform3D>(entity_id);
                shadersStorage[0].setUniform("M", transform->getTransformationMatrix()); // Model transform
            }

            // Access entity's mesh and material handle from Mesh component
            if (EM.hasComponent<MeshComponent>(entity_id)) {
                MeshComponent* mesh_comp = EM.getComponent<MeshComponent>(entity_id);

                // Update handles only if it exists
                if (mesh_comp->getMeshHandle() < meshStorage.size()) {
                    mesh_handle = mesh_comp->getMeshHandle();
                }
                if (mesh_comp->getMaterialHandle() < m_material_storage.size()) {
                    material_handle = mesh_comp->getMaterialHandle();
                }
            }

            // Set uniforms for material data
            Material& material = m_material_storage[material_handle];
            shadersStorage[0].setUniform("material.Ka", material.getMaterialAmbient());  // Ambient
            shadersStorage[0].setUniform("material.Kd", material.getMaterialDiffuse());  // Diffuse
            shadersStorage[0].setUniform("material.Ks", material.getMaterialSpecular());  // Specular
            shadersStorage[0].setUniform("material.shininess", material.getMaterialShininess());  // Shininess

            // Retrieve mesh from mesh storage
            MeshGL& mesh = meshStorage[mesh_handle];

            // Bind selected mesh
            mesh.vao.bind();

            // Draw the actual object
            glDrawElements(mesh.primitive_type, mesh.draw_count, mesh.index_type, NULL);

            // Unbind mesh
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

    std::string GraphicsManager::getMeshName(uint16_t handle) const
    {
        if (handle == 0) {
            return "Cube";
        }
        else if (handle == 1) {
            return "Plane";
        }
        else if (handle == 2) {
            return "Sphere";
        }
        else if (handle < meshStorage.size()) {
            // For any future custom meshes loaded beyond the built-in 3
            return "Custom Mesh " + std::to_string(handle);
        }
        else {
            // Invalid handle
            return "Invalid Mesh";
        }
    }
    Material* GraphicsManager::getMaterial(uint16_t handle)
    {
        auto it = m_material_storage.find(handle);
        if (it != m_material_storage.end()) {
            return const_cast<Material*>(&it->second);
        }
        return nullptr;
    }

    std::string GraphicsManager::getMeshGUID(uint16_t handle) const
    {
        if (handle == 0)
        {
            return "builtin:cube";
        }
        else if (handle == 1)
        {
            return "builtin:plane";
        }
        else if (handle == 2)
        {
            return "builtin:sphere";
        }
        else if (handle < meshStorage.size()) {
            
            return "custom:mesh_" + std::to_string(handle);
        }
        else {
            return "";  // Invalid
        }
    }
 

} // end of namespace gam300