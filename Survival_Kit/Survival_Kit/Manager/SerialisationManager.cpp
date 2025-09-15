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
#include "../Utility/InputKeyMappings.h"
#include <fstream>
#include <sstream>
#include <functional>

namespace gam300 {

    // InputComponentSerializer implementation
    std::string InputComponentSerializer::serialize(Component* component) {
        InputComponent* input = static_cast<InputComponent*>(component);
        if (!input) {
            return "{}";
        }

        std::stringstream ss;
        ss << "{\n";

        // Get the actual mappings from the InputComponent
        const auto& actions = input->getActionMappings();

        // Separate keyboard and mouse mappings
        std::vector<const InputAction*> keyMappings;
        std::vector<const InputAction*> mouseMappings;

        // Sort the actions into the appropriate categories
        for (const auto& pair : actions) {
            const InputAction& action = pair.second;
            // Mouse buttons start at GLFW_MOUSE_BUTTON_1
            if (action.input_key >= GLFW_MOUSE_BUTTON_1) {
                mouseMappings.push_back(&action);
            }
            else {
                keyMappings.push_back(&action);
            }
        }

        // Serialize key mappings
        ss << "          \"keyMappings\": [\n";
        for (size_t i = 0; i < keyMappings.size(); i++) {
            const InputAction* action = keyMappings[i];
            ss << "            {\n";
            ss << "              \"name\": \"" << action->name << "\",\n";

            // Convert action type to string
            std::string typeStr = "press";
            if (action->type == InputActionType::RELEASE) typeStr = "release";
            else if (action->type == InputActionType::REPEAT) typeStr = "repeat";
            else if (action->type == InputActionType::AXIS) typeStr = "axis";

            ss << "              \"type\": \"" << typeStr << "\",\n";

            // Convert key code to string
            std::string keyStr = "UNKNOWN";
            for (const auto& keyPair : getKeyNameMap()) {
                if (keyPair.second == action->input_key) {
                    keyStr = keyPair.first;
                    break;
                }
            }

            ss << "              \"key\": \"" << keyStr << "\",\n";
            ss << "              \"action\": \"" << action->name << " action\"\n";
            ss << "            }";

            // Add comma if not the last item
            if (i < keyMappings.size() - 1) {
                ss << ",";
            }
            ss << "\n";
        }
        ss << "          ],\n";

        // Serialize mouse mappings
        ss << "          \"mouseMappings\": [\n";
        for (size_t i = 0; i < mouseMappings.size(); i++) {
            const InputAction* action = mouseMappings[i];
            ss << "            {\n";
            ss << "              \"name\": \"" << action->name << "\",\n";

            // Convert action type to string
            std::string typeStr = "press";
            if (action->type == InputActionType::RELEASE) typeStr = "release";
            else if (action->type == InputActionType::REPEAT) typeStr = "repeat";

            ss << "              \"type\": \"" << typeStr << "\",\n";

            // Convert button code to string
            std::string buttonStr = "UNKNOWN";
            int buttonIndex = action->input_key - GLFW_MOUSE_BUTTON_1; // Convert back to button index
            for (const auto& buttonPair : getMouseButtonNameMap()) {
                if (buttonPair.second == buttonIndex) {
                    buttonStr = buttonPair.first;
                    break;
                }
            }

            ss << "              \"button\": \"" << buttonStr << "\",\n";
            ss << "              \"action\": \"" << action->name << " action\"\n";
            ss << "            }";

            // Add comma if not the last item
            if (i < mouseMappings.size() - 1) {
                ss << ",";
            }
            ss << "\n";
        }
        ss << "          ]\n";
        ss << "        }";

        return ss.str();
    }

    // InputComponentDeserializer implementation
    Component* InputComponentSerializer::deserialize(EntityID entityId, const std::string& jsonData) {
        // Create the InputComponent
        InputComponent* input = EM.addComponent<InputComponent>(entityId);

        // Parse key mappings
        std::string keyMappingsSection = SerialisationManager::extractSection(jsonData, "\"keyMappings\"");
        if (!keyMappingsSection.empty()) {
            SerialisationManager::parseKeyMappings(keyMappingsSection, input);
        }

        // Parse mouse mappings
        std::string mouseMappingsSection = SerialisationManager::extractSection(jsonData, "\"mouseMappings\"");
        if (!mouseMappingsSection.empty()) {
            SerialisationManager::parseMouseMappings(mouseMappingsSection, input);
        }

        return input;
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
        registerComponentSerializer("Input", std::make_shared<InputComponentSerializer>());

        // Register component creators
        registerComponentCreator("Input", [this](EntityID entityId, const std::string& componentData) {
            // Use the serializer to create the component
            auto serializer = m_component_serializers["Input"];
            if (serializer) {
                serializer->deserialize(entityId, componentData);
                LM.writeLog("InputComponent created for entity %d", entityId);
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
            return false;
        }

        // Very simple JSON parsing - in a real implementation we would use a proper JSON parser
        // Find the objects array
        /*size_t objectsStart = fileContent.find("\"objects\"");
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
        }*/

        // Extract the objects array content
        std::string objectsContent = fileContent; //.substr(arrayStart + 1, arrayEnd - arrayStart - 1);

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

            // Check for Input component
            if (auto serializer = m_component_serializers.find("Input");
                serializer != m_component_serializers.end()) {
                if (InputComponent* input = EM.getComponent<InputComponent>(entity.get_id())) {
                    file << getIndent(4) << "\"Input\": " << serializer->second->serialize(input);
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

    // Parse key mappings
    void SerialisationManager::parseKeyMappings(const std::string& keyMappingsJson, InputComponent* input) {
        // Split the array into individual objects
        std::vector<std::string> mappings = splitJsonArray(keyMappingsJson);

        for (const auto& mapping : mappings) {
            std::string name = extractQuotedValue(mapping, "name");
            std::string type = extractQuotedValue(mapping, "type");
            std::string key = extractQuotedValue(mapping, "key");
            std::string action = extractQuotedValue(mapping, "action");

            // Skip invalid mappings
            if (name.empty() || type.empty() || key.empty()) {
                continue;
            }

            // Convert key string to GLFW code
            int keyCode = getKeyCodeFromName(key);
            if (keyCode == GLFW_KEY_UNKNOWN) {
                LM.writeLog("Warning: Unknown key '%s' in mapping '%s'", key.c_str(), name.c_str());
                continue;
            }

            // Create a closure that logs the action
            std::string actionCopy = action; // Copy for the lambda capture
            auto callback = [actionCopy]() {
                LM.writeLog("%s", actionCopy.c_str());
                };

            // Register the mapping based on type
            if (type == "press") {
                input->mapKeyPress(name, keyCode, callback);
                LM.writeLog("Added key press mapping: %s -> %s", name.c_str(), action.c_str());
            }
            else if (type == "release") {
                input->mapKeyRelease(name, keyCode, callback);
                LM.writeLog("Added key release mapping: %s -> %s", name.c_str(), action.c_str());
            }
            else if (type == "repeat") {
                input->mapKeyRepeat(name, keyCode, callback);
                LM.writeLog("Added key repeat mapping: %s -> %s", name.c_str(), action.c_str());
            }
        }
    }

    // Parse mouse mappings
    void SerialisationManager::parseMouseMappings(const std::string& mouseMappingsJson, InputComponent* input) {
        // Split the array into individual objects
        std::vector<std::string> mappings = splitJsonArray(mouseMappingsJson);

        for (const auto& mapping : mappings) {
            std::string name = extractQuotedValue(mapping, "name");
            std::string type = extractQuotedValue(mapping, "type");
            std::string button = extractQuotedValue(mapping, "button");
            std::string action = extractQuotedValue(mapping, "action");

            // Skip invalid mappings
            if (name.empty() || type.empty() || button.empty()) {
                continue;
            }

            // Convert button string to GLFW code
            int buttonCode = getMouseButtonFromName(button);
            if (buttonCode == -1) {
                LM.writeLog("Warning: Unknown mouse button '%s' in mapping '%s'", button.c_str(), name.c_str());
                continue;
            }

            // Create a closure that logs the action
            std::string actionCopy = action; // Copy for the lambda capture
            auto callback = [actionCopy]() {
                LM.writeLog("%s", actionCopy.c_str());
                };

            // Register the mapping based on type
            if (type == "press") {
                input->mapMousePress(name, buttonCode, callback);
                LM.writeLog("Added mouse press mapping: %s -> %s", name.c_str(), action.c_str());
            }
            else if (type == "release") {
                input->mapMouseRelease(name, buttonCode, callback);
                LM.writeLog("Added mouse release mapping: %s -> %s", name.c_str(), action.c_str());
            }
        }
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

} // end of namespace gam300