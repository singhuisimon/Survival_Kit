/**
 * @file ShaderProgram.h
 * @brief Declaration of shader program class used in the game engine for rendering scenes.
 * @details Manages the creation and usage of shader programs.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once
#ifndef __SHADERPROGRAM_H__
#define __SHADERPROGRAM_H__

// Includes
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include "../Utility/Constant.h"

// For logging information
#include "../Manager/LogManager.h"

namespace gam300 {

    class ShaderProgram {

    private:

        GLuint program_handle = 0;
        GLboolean link_status = GL_FALSE; 

    public:

        /**
        * @brief Compile the shaders, link the shader objects to create an executable,
                 and ensure the program can work in the current OpenGL state.
        * @param shader_files The data which contains the shader type and its filepath.
        * @param shader The shader program that will be created, compiled, and link.
        * @return True if shader program compile and link successfully, false otherwise.
        */
        GLboolean compileShader(std::vector<std::pair<GLenum, std::string>> shader_files);

        // Reading shader files
        bool readShaderFile(const std::string& file_path, std::string& shader_source);

        /**
         * @brief Start the shader program
         *
         *.@param shader The shader program that is to be started
         */
        void programUse();

        /**
         * @brief Free the shader program
         */
        void programFree();

        /**
         * @brief Return the shader program handle
         *
         * @return The program handle
         */
        GLuint getShaderProgramHandle() const;

        /**
         * @brief Return the shader program link status
         *
         * @return The program link status
         */
        GLuint getShaderProgramLinkStatus() const;

        // Bind object's VAO handle

        /******************** Functions for setting uniforms ********************/

        // Set uniform for type vec2 (float x, y)
        void setUniform(const std::string& name, float x, float y);

        // Set uniform for type vec3 (float x, y, z)
        void setUniform(const std::string& name, float x, float y, float z);

        // Set uniform for type vec4 (float x, y, z, w)
        void setUniform(const std::string& name, float x, float y, float z, float w);

        // Set uniform for type vec2 (glm::vec2)
        void setUniform(const std::string& name, glm::vec2 v, GLsizei cnt = 1);

        // Set uniform for type vec3 (glm::vec3)
        void setUniform(const std::string& name, glm::vec3 v, GLsizei cnt = 1);

        // Set uniform for type vec4 (glm::vec4)
        void setUniform(const std::string& name, glm::vec4 v, GLsizei cnt = 1);

        // Set uniform for type mat3 (glm::mat3)
        void setUniform(const std::string& name, glm::mat3 mat, GLsizei cnt = 1);

        // Set uniform for type mat4 (glm::mat4)
        void setUniform(const std::string& name, glm::mat4 mat, GLsizei cnt = 1);

        // Set uniform for float
        void setUniform(const std::string& name, float val);

        // Set uniform for integer
        void setUniform(const std::string& name, int val);

        // Set uniform for unsigned integer
        void setUniform(const std::string& name, GLuint val);

        // P.S. -> Set variable name as such: "uIndices[0]" instead of "uIndices"
        void setUniform(const std::string& name, const GLuint *val, GLsizei cnt = 1);

        // Set uniform for boolean
        void setUniform(const std::string& name, GLboolean val);

    };

} // end of namespace gam300
#endif // __SHADERPROGRAM_H__
