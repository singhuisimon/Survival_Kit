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
#include "../Component/Transform3D.h"
#include <fstream>
#include <sstream>
#include <functional>

namespace gam300 {

    // Transform3DSerializer implementation
    std::string Transform3DSerializer::serialize(Component* component) {
        Transform3D* transform = static_cast<Transform3D*>(component);
        if (!transform) {
            return "{}";
        }

        std::stringstream ss;
        ss << "{\n";

        // Serialize position
        const Vector3D& pos = transform->getPosition();
        ss << "          \"position\": [\n";
        ss << "            " << pos.x << ",\n";
        ss << "            " << pos.y << ",\n";
        ss << "            " << pos.z << "\n";
        ss << "          ],\n";

        // Serialize previous position
        const Vector3D& prevPos = transform->getPrevPosition();
        ss << "          \"prev_position\": [\n";
        ss << "            " << prevPos.x << ",\n";
        ss << "            " << prevPos.y << ",\n";
        ss << "            " << prevPos.z << "\n";
        ss << "          ],\n";

        // Serialize rotation
        const Vector3D& rotation = transform->getRotation();
        ss << "          \"rotation\": [\n";
        ss << "            " << rotation.x << ",\n";
        ss << "            " << rotation.y << ",\n";
        ss << "            " << rotation.z << "\n";
        ss << "          ],\n";

        // Serialize scale
        const Vector3D& scale = transform->getScale();
        ss << "          \"scale\": [\n";
        ss << "            " << scale.x << ",\n";
        ss << "            " << scale.y << ",\n";
        ss << "            " << scale.z << "\n";
        ss << "          ]\n";

        ss << "        }";

        return ss.str();
    }

    // Transform3DDeserializer implementation
    Component* Transform3DSerializer::deserialize(EntityID entityId, const std::string& jsonData) {
        // Parse position
        Vector3D position = Vector3D::ZERO;
        std::string positionData = SerialisationManager::extractObjectValue(jsonData, "position");
        if (!positionData.empty()) {
            std::vector<float> posArray = SerialisationManager::parseFloatArray(positionData);
            if (posArray.size() >= 3) {
                position = Vector3D(posArray[0], posArray[1], posArray[2]);
            }
        }

        // Parse rotation
        Vector3D rotation = Vector3D::ZERO;
        std::string rotationData = SerialisationManager::extractObjectValue(jsonData, "rotation");
        if (!rotationData.empty()) {
            std::vector<float> rotArray = SerialisationManager::parseFloatArray(rotationData);
            if (rotArray.size() >= 3) {
                rotation = Vector3D(rotArray[0], rotArray[1], rotArray[2]);
            }
        }

        // Parse scale
        Vector3D scale = Vector3D::ONE;
        std::string scaleData = SerialisationManager::extractObjectValue(jsonData, "scale");
        if (!scaleData.empty()) {
            std::vector<float> scaleArray = SerialisationManager::parseFloatArray(scaleData);
            if (scaleArray.size() >= 3) {
                scale = Vector3D(scaleArray[0], scaleArray[1], scaleArray[2]);
            }
        }

        // Create the Transform3D component
        Transform3D* transform = EM.addComponent<Transform3D>(entityId, position, rotation, scale);

        // Parse and set previous position if available
        std::string prevPosData = SerialisationManager::extractObjectValue(jsonData, "prev_position");
        if (!prevPosData.empty()) {
            std::vector<float> prevPosArray = SerialisationManager::parseFloatArray(prevPosData);
            if (prevPosArray.size() >= 3) {
                // Since there's no direct setter for prev_position, we'll just log it
                // In a real implementation, you might add a setPrevPosition method
                LM.writeLog("Transform3D::deserialize() - Previous position loaded but not set (no direct setter)");
            }
        }

        return transform;
    }

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

        // Register component serializers
        registerComponentSerializer("Transform3D", std::make_shared<Transform3DSerializer>());

        // Register component creators
        registerComponentCreator("Transform3D", [this](EntityID entityId, const std::string& componentData) {
            // Use the serializer to create the component
            auto serializer = m_component_serializers["Transform3D"];
            if (serializer) {
                serializer->deserialize(entityId, componentData);
                LM.writeLog("Transform3D created for entity %d", entityId);
            }
            });

        // Log startup
        LM.writeLog("SerialisationManager::startUp() - Serialisation Manager started successfully");

        return 0;
    }

    // Shut down the SerialisationManager
    void SerialisationManager::shutDown() {
        // Log shutdown
        LM.writeLog("SerialisationManager::shutDown() - Shutting down Serialisation Manager");

        // Clear component creators and serializers
        m_component_creators.clear();
        m_component_serializers.clear();

        // Call parent's shutDown()
        Manager::shutDown();
    }

    // Register a component creator function
    void SerialisationManager::registerComponentCreator(const std::string& componentName, ComponentCreatorFunc creatorFunc) {
        m_component_creators[componentName] = creatorFunc;
        LM.writeLog("SerialisationManager::registerComponentCreator() - Registered creator for '%s'", componentName.c_str());
    }

    // Register a component serializer
    void SerialisationManager::registerComponentSerializer(const std::string& componentName, std::shared_ptr<IComponentSerializer> serializer) {
        m_component_serializers[componentName] = serializer;
        LM.writeLog("SerialisationManager::registerComponentSerializer() - Registered serializer for '%s'", componentName.c_str());
    }

	// Load entities from a scene file
    bool SerialisationManager::loadScene(const std::string& filename) {
        LM.writeLog("SerialisationManager::loadScene() - Loading scene from '%s'", filename.c_str());

        // Read file content
        std::string fileContent;
        if (!parseJsonFile(filename, fileContent)) {
            LM.writeLog("SerialisationManager::loadScene() - Failed to read scene file");
            return false; // PROPER ERROR RETURN
        }

        LM.writeLog("SerialisationManager::loadScene() - File loaded, size: %zu characters", fileContent.length());

        // Simple approach: Find each occurrence of a named entity
        size_t searchPos = 0;
        int entityCount = 0;
        bool foundAnyEntities = false;

        while (true) {
            // Find next "name" field
            size_t namePos = fileContent.find("\"name\"", searchPos);
            if (namePos == std::string::npos) {
                break; // No more entities
            }

            // Extract the entity name
            size_t colonPos = fileContent.find(':', namePos);
            size_t nameStartQuote = fileContent.find('"', colonPos);
            size_t nameEndQuote = fileContent.find('"', nameStartQuote + 1);

            if (colonPos == std::string::npos || nameStartQuote == std::string::npos || nameEndQuote == std::string::npos) {
                LM.writeLog("SerialisationManager::loadScene() - Malformed name field at position %zu", namePos);
                searchPos = namePos + 1;
                continue;
            }

            std::string entityName = fileContent.substr(nameStartQuote + 1, nameEndQuote - nameStartQuote - 1);
            LM.writeLog("SerialisationManager::loadScene() - Found entity: '%s'", entityName.c_str());

            // Create the entity
            Entity& entity = EM.createEntity(entityName);
            entityCount++;
            foundAnyEntities = true;
            LM.writeLog("SerialisationManager::loadScene() - Created entity '%s' with ID %d", entityName.c_str(), entity.get_id());

            // Look for Transform3D component
            size_t transform3DPos = fileContent.find("\"Transform3D\"", namePos);
            size_t nextNamePos = fileContent.find("\"name\"", namePos + 1);

            // Make sure this Transform3D belongs to this entity (not the next one)
            if (transform3DPos != std::string::npos &&
                (nextNamePos == std::string::npos || transform3DPos < nextNamePos)) {

                LM.writeLog("SerialisationManager::loadScene() - Found Transform3D for entity '%s'", entityName.c_str());

                // Extract Transform3D data using simple string search
                Vector3D position = extractVector3D(fileContent, transform3DPos, "position");
                Vector3D rotation = extractVector3D(fileContent, transform3DPos, "rotation");
                Vector3D scale = extractVector3D(fileContent, transform3DPos, "scale");

                LM.writeLog("SerialisationManager::loadScene() - Position: (%.1f, %.1f, %.1f)", position.x, position.y, position.z);
                LM.writeLog("SerialisationManager::loadScene() - Rotation: (%.1f, %.1f, %.1f)", rotation.x, rotation.y, rotation.z);
                LM.writeLog("SerialisationManager::loadScene() - Scale: (%.1f, %.1f, %.1f)", scale.x, scale.y, scale.z);

                // Create the Transform3D component
                Transform3D* transform = EM.addComponent<Transform3D>(entity.get_id(), position, rotation, scale);
                if (transform) {
                    LM.writeLog("SerialisationManager::loadScene() - Transform3D component created successfully for entity '%s'", entityName.c_str());
                }
                else {
                    LM.writeLog("SerialisationManager::loadScene() - Failed to create Transform3D component for entity '%s'", entityName.c_str());
                }
            }
            else {
                LM.writeLog("SerialisationManager::loadScene() - No Transform3D component found for entity '%s'", entityName.c_str());
            }

            // Move search position past this entity
            searchPos = namePos + 1;
        }

        //  PROPER SUCCESS/FAILURE LOGIC
        if (!foundAnyEntities) {
            LM.writeLog("SerialisationManager::loadScene() - ERROR: No entities found in scene file");
            return false; // RETURN FALSE IF NO ENTITIES LOADED
        }

        LM.writeLog("SerialisationManager::loadScene() - Scene loaded successfully, processed %d entities", entityCount);
        return true; // ONLY RETURN TRUE IF ENTITIES WERE ACTUALLY LOADED
    }

    // Save current entities to a scene file
    bool SerialisationManager::saveScene(const std::string& filename) {
        LM.writeLog("SerialisationManager::saveScene() - Saving scene to '%s'", filename.c_str());

        // Create the scene file
        std::ofstream file(filename);
        if (!file.is_open()) {
            LM.writeLog("SerialisationManager::saveScene() - Failed to open file for writing");
            return false;
        }

        // Get all entities
        const auto& entities = EM.getAllEntities();

        // Start the JSON structure
        file << "{\n";
        file << getIndent(1) << "\"objects\": [\n";

        // Save each entity
        for (size_t i = 0; i < entities.size(); ++i) {
            const Entity& entity = entities[i];

            file << getIndent(2) << "{\n";
            file << getIndent(3) << "\"name\": \"" << entity.get_name() << "\",\n";
            file << getIndent(3) << "\"components\": {\n";

            // Save each component
            bool hasComponents = false;

            // Check for Transform3D component
            if (auto serializer = m_component_serializers.find("Transform3D");
                serializer != m_component_serializers.end()) {
                if (Transform3D* transform = EM.getComponent<Transform3D>(entity.get_id())) {
                    file << getIndent(4) << "\"Transform3D\": " << serializer->second->serialize(transform);
                    hasComponents = true;
                }
            }

            // TODO: Add more component types here as needed

            // Close the components object
            file << "\n" << getIndent(3) << "}";

            // Add comma if not the last entity
            file << (i < entities.size() - 1 ? "," : "") << "\n";
            file << getIndent(2) << "}";
            file << (i < entities.size() - 1 ? "," : "") << "\n";
        }

        // Close the JSON structure
        file << getIndent(1) << "]\n";
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

    // Helper function to extract a section from JSON
    std::string SerialisationManager::extractSection(const std::string& json, const std::string& sectionName) {
        size_t pos = json.find(sectionName);
        if (pos == std::string::npos) {
            return ""; // Section not found
        }

        // Find the beginning of the array
        size_t arrayStart = json.find('[', pos);
        if (arrayStart == std::string::npos) {
            return ""; // Array not found
        }

        // Find the end of the array, accounting for nested arrays
        int bracketLevel = 1;
        size_t arrayEnd = arrayStart + 1;

        while (bracketLevel > 0 && arrayEnd < json.length()) {
            if (json[arrayEnd] == '[') {
                bracketLevel++;
            }
            else if (json[arrayEnd] == ']') {
                bracketLevel--;
            }
            arrayEnd++;
        }

        if (bracketLevel != 0) {
            return ""; // Unbalanced brackets
        }

        return json.substr(arrayStart, arrayEnd - arrayStart);
    }

    // Helper function to extract quoted string
    std::string SerialisationManager::extractQuotedValue(const std::string& json, const std::string& fieldName) {
        size_t pos = json.find("\"" + fieldName + "\"");
        if (pos == std::string::npos) {
            return ""; // Field not found
        }

        // Find the colon after the field name
        size_t colonPos = json.find(':', pos);
        if (colonPos == std::string::npos) {
            return ""; // Invalid JSON format
        }

        // Find the beginning of the string value
        size_t valueStart = json.find('"', colonPos);
        if (valueStart == std::string::npos) {
            return ""; // Invalid JSON format
        }

        // Find the end of the string value
        size_t valueEnd = json.find('"', valueStart + 1);
        if (valueEnd == std::string::npos) {
            return ""; // Invalid JSON format
        }

        return json.substr(valueStart + 1, valueEnd - valueStart - 1);
    }

    // Helper function to extract object/array value
    std::string SerialisationManager::extractObjectValue(const std::string& json, const std::string& fieldName) {
        size_t pos = json.find("\"" + fieldName + "\"");
        if (pos == std::string::npos) {
            return ""; // Field not found
        }

        // Find the colon after the field name
        size_t colonPos = json.find(':', pos);
        if (colonPos == std::string::npos) {
            return ""; // Invalid JSON format
        }

        // Find the beginning of the array
        size_t arrayStart = json.find('[', colonPos);
        if (arrayStart == std::string::npos) {
            return ""; // Array not found
        }

        // Find the end of the array, accounting for nested arrays
        int bracketLevel = 1;
        size_t arrayEnd = arrayStart + 1;

        while (bracketLevel > 0 && arrayEnd < json.length()) {
            if (json[arrayEnd] == '[') {
                bracketLevel++;
            }
            else if (json[arrayEnd] == ']') {
                bracketLevel--;
            }
            arrayEnd++;
        }

        if (bracketLevel != 0) {
            return ""; // Unbalanced brackets
        }

        return json.substr(arrayStart, arrayEnd - arrayStart);
    }

    // Helper function to parse float array
    std::vector<float> SerialisationManager::parseFloatArray(const std::string& arrayJson) {
        std::vector<float> result;

        // Remove brackets and spaces
        std::string content = arrayJson;
        if (content.front() == '[') content = content.substr(1);
        if (content.back() == ']') content.pop_back();

        // Split by comma and parse floats
        std::stringstream ss(content);
        std::string token;

        while (std::getline(ss, token, ',')) {
            // Remove leading/trailing whitespace
            size_t start = token.find_first_not_of(" \t\n\r");
            size_t end = token.find_last_not_of(" \t\n\r");

            if (start != std::string::npos && end != std::string::npos) {
                token = token.substr(start, end - start + 1);
                try {
                    result.push_back(std::stof(token));
                }
                catch (const std::exception&) {
                    // Skip invalid numbers
                }
            }
        }

        return result;
    }

    // Split a JSON array into individual objects
    std::vector<std::string> SerialisationManager::splitJsonArray(const std::string& jsonArray) {
        std::vector<std::string> result;

        // Find each object in the array
        size_t pos = 0;
        while (pos < jsonArray.length()) {
            // Find the start of an object
            size_t objectStart = jsonArray.find('{', pos);
            if (objectStart == std::string::npos) {
                break; // No more objects
            }

            // Find the end of the object, accounting for nested objects
            int braceLevel = 1;
            size_t objectEnd = objectStart + 1;

            while (braceLevel > 0 && objectEnd < jsonArray.length()) {
                if (jsonArray[objectEnd] == '{') {
                    braceLevel++;
                }
                else if (jsonArray[objectEnd] == '}') {
                    braceLevel--;
                }
                objectEnd++;
            }

            if (braceLevel != 0) {
                break; // Unbalanced braces
            }

            // Extract the object and add it to the result
            std::string object = jsonArray.substr(objectStart, objectEnd - objectStart);
            result.push_back(object);

            // Move to the next position
            pos = objectEnd;
        }

        return result;
    }

    // Helper for JSON indentation
    std::string SerialisationManager::getIndent(int level) const {
        std::string indent;
        for (int i = 0; i < level; ++i) {
            indent += "  "; // Two spaces per level
        }
        return indent;
    }

    Vector3D SerialisationManager::extractVector3D(const std::string& json, size_t startPos, const std::string& fieldName) {
        // Find the field
        size_t fieldPos = json.find("\"" + fieldName + "\"", startPos);
        if (fieldPos == std::string::npos) {
            LM.writeLog("SerialisationManager::extractVector3D() - Field '%s' not found", fieldName.c_str());
            return Vector3D::ZERO;
        }

        // Find the opening bracket
        size_t bracketStart = json.find('[', fieldPos);
        if (bracketStart == std::string::npos) {
            return Vector3D::ZERO;
        }

        // Find the closing bracket
        size_t bracketEnd = json.find(']', bracketStart);
        if (bracketEnd == std::string::npos) {
            return Vector3D::ZERO;
        }

        // Extract array content
        std::string arrayContent = json.substr(bracketStart + 1, bracketEnd - bracketStart - 1);

        // Parse the three float values
        std::vector<float> values;
        std::stringstream ss(arrayContent);
        std::string token;

        while (std::getline(ss, token, ',')) {
            // Remove whitespace
            size_t start = token.find_first_not_of(" \t\n\r");
            size_t end = token.find_last_not_of(" \t\n\r");

            if (start != std::string::npos && end != std::string::npos) {
                token = token.substr(start, end - start + 1);
                try {
                    values.push_back(std::stof(token));
                }
                catch (const std::exception&) {
                    LM.writeLog("SerialisationManager::extractVector3D() - Failed to parse float: '%s'", token.c_str());
                    values.push_back(0.0f);
                }
            }
        }

        // Ensure we have at least 3 values
        while (values.size() < 3) {
            values.push_back(0.0f);
        }

        return Vector3D(values[0], values[1], values[2]);
    }

} // end of namespace gam300