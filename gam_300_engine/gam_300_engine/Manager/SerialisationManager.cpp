/**
 * @file SerialisationManager.cpp
 * @brief Implementation of the Serialisation Manager for the game engine.
 * @details Handles loading and saving game objects to/from files in .scn format.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#include "SerialisationManager.h"
#include "LogManager.h"
#include "ECSManager.h"
#include "../Component/InputComponent.h"
#include <fstream>
#include <sstream>

namespace gam300 {

    // Initialize singleton instance
    SerialisationManager::SerialisationManager() {
        setType("SerialisationManager");
    }

    // Get the singleton instance
    SerialisationManager& SerialisationManager::getInstance() {
        static SerialisationManager instance;
        return instance;
    }

    // Start up the SerialisationManager
    int SerialisationManager::startUp() {
        // Call parent's startUp() first
        if (Manager::startUp())
            return -1;

        // Register component creators
        registerComponentCreator("Input", [](EntityID entityId, const std::string& componentData) {
            // Mark parameter as unused to avoid compiler warning
            (void)componentData;  // Explicitly tell compiler that this parameter is intentionally unused

            // Create InputComponent
            InputComponent* input = EM.addComponent<InputComponent>(entityId);

            // Set up basic input mappings for the player
            input->mapKeyPress("move_up", GLFW_KEY_W, []() {
                LM.writeLog("Player moving up");
                });

            input->mapKeyPress("move_down", GLFW_KEY_S, []() {
                LM.writeLog("Player moving down");
                });

            input->mapKeyPress("move_left", GLFW_KEY_A, []() {
                LM.writeLog("Player moving left");
                });

            input->mapKeyPress("move_right", GLFW_KEY_D, []() {
                LM.writeLog("Player moving right");
                });

            // Add some mouse input as well
            input->mapMousePress("primary_action", GLFW_MOUSE_BUTTON_LEFT, []() {
                LM.writeLog("Player primary action");
                });

            input->mapMousePress("secondary_action", GLFW_MOUSE_BUTTON_RIGHT, []() {
                LM.writeLog("Player secondary action");
                });

            LM.writeLog("InputComponent created for entity %d", entityId);
            });

        // Log startup
        LM.writeLog("SerialisationManager::startUp() - Serialisation Manager started successfully");

        return 0;
    }

    // Shut down the SerialisationManager
    void SerialisationManager::shutDown() {
        // Log shutdown
        LM.writeLog("SerialisationManager::shutDown() - Shutting down Serialisation Manager");

        // Clear component creators
        m_component_creators.clear();

        // Call parent's shutDown()
        Manager::shutDown();
    }

    // Register a component creator function
    void SerialisationManager::registerComponentCreator(const std::string& componentName, ComponentCreatorFunc creatorFunc) {
        m_component_creators[componentName] = creatorFunc;
        LM.writeLog("SerialisationManager::registerComponentCreator() - Registered creator for '%s'", componentName.c_str());
    }

    // Load entities from a scene file
    bool SerialisationManager::loadScene(const std::string& filename) {
        LM.writeLog("SerialisationManager::loadScene() - Loading scene from '%s'", filename.c_str());

        // Read file content
        std::string fileContent;
        if (!parseJsonFile(filename, fileContent)) {
            LM.writeLog("SerialisationManager::loadScene() - Failed to read scene file");
            return false;
        }

        // Very simple JSON parsing - in a real implementation we would use a proper JSON parser
        // Find the objects array
        size_t objectsStart = fileContent.find("\"objects\"");
        if (objectsStart == std::string::npos) {
            LM.writeLog("SerialisationManager::loadScene() - No objects found in scene file");
            return false;
        }

        // Find the beginning of the objects array
        size_t arrayStart = fileContent.find('[', objectsStart);
        if (arrayStart == std::string::npos) {
            LM.writeLog("SerialisationManager::loadScene() - Invalid objects format in scene file");
            return false;
        }

        // Find the end of the objects array
        size_t arrayEnd = fileContent.find(']', arrayStart);
        if (arrayEnd == std::string::npos) {
            LM.writeLog("SerialisationManager::loadScene() - Invalid objects format in scene file");
            return false;
        }

        // Extract the objects array content
        std::string objectsContent = fileContent.substr(arrayStart + 1, arrayEnd - arrayStart - 1);

        // Process each object in the array
        size_t objectStart = 0;
        while (objectStart < objectsContent.length()) {
            // Find the beginning of the object
            objectStart = objectsContent.find('{', objectStart);
            if (objectStart == std::string::npos) {
                break; // No more objects
            }

            // Find the end of the object
            int braceLevel = 1;
            size_t objectEnd = objectStart + 1;
            while (braceLevel > 0 && objectEnd < objectsContent.length()) {
                if (objectsContent[objectEnd] == '{') {
                    braceLevel++;
                }
                else if (objectsContent[objectEnd] == '}') {
                    braceLevel--;
                }
                objectEnd++;
            }

            if (braceLevel != 0) {
                LM.writeLog("SerialisationManager::loadScene() - Invalid object format in scene file");
                break;
            }

            objectEnd--; // Move back to the closing brace

            // Extract the object content
            std::string objectContent = objectsContent.substr(objectStart, objectEnd - objectStart + 1);

            // Find the name of the object
            size_t nameStart = objectContent.find("\"name\"");
            if (nameStart == std::string::npos) {
                LM.writeLog("SerialisationManager::loadScene() - Object without name in scene file");
                objectStart = objectEnd + 1;
                continue;
            }

            // Extract the name
            size_t nameValueStart = objectContent.find(':', nameStart);
            if (nameValueStart == std::string::npos) {
                objectStart = objectEnd + 1;
                continue;
            }

            size_t nameStrStart = objectContent.find('"', nameValueStart);
            if (nameStrStart == std::string::npos) {
                objectStart = objectEnd + 1;
                continue;
            }

            size_t nameStrEnd = objectContent.find('"', nameStrStart + 1);
            if (nameStrEnd == std::string::npos) {
                objectStart = objectEnd + 1;
                continue;
            }

            std::string entityName = objectContent.substr(nameStrStart + 1, nameStrEnd - nameStrStart - 1);

            // Create the entity
            Entity& entity = EM.createEntity(entityName);
            LM.writeLog("SerialisationManager::loadScene() - Created entity '%s' with ID %d", entityName.c_str(), entity.get_id());

            // Find the components section
            size_t componentsStart = objectContent.find("\"components\"");
            if (componentsStart != std::string::npos) {
                // Extract the components content
                size_t componentsBraceStart = objectContent.find('{', componentsStart);
                if (componentsBraceStart != std::string::npos) {
                    // Find the closing brace of the components object, accounting for nested braces
                    int componentBraceLevel = 1;
                    size_t componentsBraceEnd = componentsBraceStart + 1;
                    while (componentBraceLevel > 0 && componentsBraceEnd < objectContent.length()) {
                        if (objectContent[componentsBraceEnd] == '{') {
                            componentBraceLevel++;
                        }
                        else if (objectContent[componentsBraceEnd] == '}') {
                            componentBraceLevel--;
                        }
                        componentsBraceEnd++;
                    }

                    if (componentBraceLevel == 0) {
                        componentsBraceEnd--; // Move back to the closing brace
                        std::string componentsContent = objectContent.substr(componentsBraceStart, componentsBraceEnd - componentsBraceStart + 1);

                        // Process the components
                        parseComponents(entity.get_id(), componentsContent);
                    }
                }
            }

            // Move to the next object
            objectStart = objectEnd + 1;
        }

        LM.writeLog("SerialisationManager::loadScene() - Scene loaded successfully");
        return true;
    }

    // Save current entities to a scene file
    bool SerialisationManager::saveScene(const std::string& filename) {
        LM.writeLog("SerialisationManager::saveScene() - Saving scene to '%s'", filename.c_str());

        // Create a simple example file
        std::ofstream file(filename);
        if (!file.is_open()) {
            LM.writeLog("SerialisationManager::saveScene() - Failed to open file for writing");
            return false;
        }

        file << "{\n";
        file << "  \"objects\": [\n";
        file << "    {\n";
        file << "      \"name\": \"player\",\n";
        file << "      \"components\": {\n";
        file << "        \"Input\": {\n";
        file << "        }\n";
        file << "      }\n";
        file << "    }\n";
        file << "  ]\n";
        file << "}\n";

        file.close();
        LM.writeLog("SerialisationManager::saveScene() - Scene saved successfully");
        return true;
    }

    // Helper method to parse a JSON file
    bool SerialisationManager::parseJsonFile(const std::string& filename, std::string& jsonContent) {
        // Open the file
        std::ifstream file(filename);
        if (!file.is_open()) {
            LM.writeLog("SerialisationManager::parseJsonFile() - Failed to open file '%s'", filename.c_str());
            return false;
        }

        // Read the entire file into a string
        std::stringstream buffer;
        buffer << file.rdbuf();
        jsonContent = buffer.str();

        // Close the file
        file.close();

        return true;
    }

    // Helper method to parse components and create them for an entity
    bool SerialisationManager::parseComponents(EntityID entityId, const std::string& componentData) {
        // For each registered component type, check if it exists in the data
        for (const auto& pair : m_component_creators) {
            const std::string& componentName = pair.first;
            const ComponentCreatorFunc& creatorFunc = pair.second;

            // Check if this component type exists in the data
            size_t componentPos = componentData.find("\"" + componentName + "\"");
            if (componentPos != std::string::npos) {
                // Extract the component data
                size_t componentBraceStart = componentData.find('{', componentPos);
                if (componentBraceStart != std::string::npos) {
                    // Find the closing brace, accounting for nested braces
                    int braceLevel = 1;
                    size_t componentBraceEnd = componentBraceStart + 1;

                    while (braceLevel > 0 && componentBraceEnd < componentData.length()) {
                        if (componentData[componentBraceEnd] == '{') {
                            braceLevel++;
                        }
                        else if (componentData[componentBraceEnd] == '}') {
                            braceLevel--;
                        }
                        componentBraceEnd++;
                    }

                    if (braceLevel == 0) {
                        componentBraceEnd--; // Move back to the closing brace
                        std::string componentContent = componentData.substr(componentBraceStart, componentBraceEnd - componentBraceStart + 1);

                        // Call the creator function with the component data
                        creatorFunc(entityId, componentContent);
                    }
                }
            }
        }

        return true;
    }

} // end of namespace gam300