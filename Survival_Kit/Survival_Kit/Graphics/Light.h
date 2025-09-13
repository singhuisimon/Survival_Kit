/**
 * @file Light.h
 * @brief Definitions of the light class that influences the lighting in the game.
 * @details Manages the lighting in the editor and game engine.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once
#ifndef __LIGHT_H__
#define __LIGHT_H__

 // Includes
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include "../Utility/Constant.h"

// For logging information
#include "../Manager/LogManager.h"


namespace gam300 {

    class Light {

    private:

        // Data members of a light
        glm::vec3 pos;              // Position of the light source
        glm::vec3 light_ambient;    // Intensity for the ambient component
        glm::vec3 light_diffuse;    // Intensity for the diffuse component
        glm::vec3 light_specular;   // Intensity for the specular component

    public:

        // Default constructor for a default light
        Light() :   pos{ 0.0f, 0.5f, 0.0f},         // From the top
                    light_ambient{ 0.4f, 0.4f, 0.4f },
                    light_diffuse{ 1.0f, 1.0f, 1.0f },
                    light_specular{ 1.0f, 1.0f, 1.0f }
        {}

        // Constructor for a light with custom values
        Light(  glm::vec3 pos,
                glm::vec3 ambient,
                glm::vec3 diffuse,
                glm::vec3 specular ) : 
            
                pos{ pos },        
                light_ambient{ ambient },
                light_diffuse{ diffuse },
                light_specular{ specular }
        {}

        // Reference to light data
        glm::vec3& getLightPos() { return pos; }
        glm::vec3& getLightAmbient() { return light_ambient; }
        glm::vec3& getLightDiffuse() { return light_diffuse; }
        glm::vec3& getLightSpecular() { return light_specular; }

    };

} // end of namespace gam300
#endif // __LIGHT_H__
