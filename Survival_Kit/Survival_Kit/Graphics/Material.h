/**
 * @file Material.h
 * @brief Definitions of the material class that contains material properties for objects in the game.
 * @details Manages the material properties in the editor and game engine.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once
#ifndef __MATERIAL_H__
#define __MATERIAL_H__

 // Includes
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include "../Utility/Constant.h"

// For logging information
#include "../Manager/LogManager.h"


namespace gam300 {

    class Material {

    private:

        // Data members of a light
        float shininess;                // Shininess of the material
        glm::vec3 material_ambient;     // Intensity for the ambient component
        glm::vec3 material_diffuse;     // Intensity for the diffuse component
        glm::vec3 material_specular;    // Intensity for the specular component

    public:

        // Default constructor for a default light
        Material() :    shininess{ 100.0f },         
                        material_ambient{ 0.9f, 0.5f, 0.3f },
                        material_diffuse{ 0.9f, 0.5f, 0.3f },
                        material_specular{ 0.8f, 0.8f, 0.8f }
        {}

        // Constructor for a light with custom values
        Material(   float shininess,
                    glm::vec3 ambient,
                    glm::vec3 diffuse,
                    glm::vec3 specular) :

                    shininess{ shininess },
                    material_ambient{ ambient },
                    material_diffuse{ diffuse },
                    material_specular{ specular }
        {}

        // Reference to light data
        float& getMaterialShininess() { return shininess; }
        glm::vec3& getMaterialAmbient() { return material_ambient; }
        glm::vec3& getMaterialDiffuse() { return material_diffuse; }
        glm::vec3& getMaterialSpecular() { return material_specular; }

    };

} // end of namespace gam300
#endif // __MATERIAL_H__
