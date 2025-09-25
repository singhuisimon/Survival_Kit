/**
 * @file SerialisationManager.h
 * @brief Declaration of the Serialisation Manager for the game engine.
 * @details Handles loading and saving game objects to/from files in .scn format.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once
#ifndef __SERIALISATION_MANAGER_H__
#define __SERIALISATION_MANAGER_H__

#include "Manager.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
#include "../Utility/ECS_Variables.h"
#include "../Utility/Vector3D.h"

 // Two-letter acronym for easier access to manager.
#define SEM gam300::SerialisationManager::getInstance()

namespace gam300 {

    // Forward declarations
    class Entity;
    class InputComponent;
    class Component;

    /**
     * @brief Interface for component serialization.
     * @details Provides methods for serializing and deserializing component data.
     */
    class IComponentSerializer {
    public:
        virtual ~IComponentSerializer() = default;

        /**
         * @brief Serialize a component to JSON format.
         * @param component Pointer to the component to serialize.
         * @return JSON string representation of the component.
         */
        virtual std::string serialize(Component* component) = 0;

        /**
         * @brief Create and configure a component from JSON data.
         * @param entityId The entity to attach the component to.
         * @param jsonData JSON data for the component.
         * @return Pointer to the created component.
         */
        virtual Component* deserialize(EntityID entityId, const std::string& jsonData) = 0;
    };

    /**
     * @brief Serializer for Transform3D components.
     */
    class Transform3DSerializer : public IComponentSerializer {
    public:
        std::string serialize(Component* component) override;
        Component* deserialize(EntityID entityId, const std::string& jsonData) override;
    };

    /**
     * @brief Serializer for Input components.
     */
    class InputComponentSerializer : public IComponentSerializer {
    public:
        std::string serialize(Component* component) override;
        Component* deserialize(EntityID entityId, const std::string& jsonData) override;
    };

    /**
     * @brief Serializer for RigidBody components.
     */
    class RigidBodySerializer : public IComponentSerializer {
    public:
        std::string serialize(Component* component) override;
        Component* deserialize(EntityID entityId, const std::string& jsonData) override;
    };
    /**
     * @brief Manager for serializing and deserializing game entities.
     * @details Handles loading entities from scene files and saving them back.
     */
    class SerialisationManager : public Manager {
    private:
        SerialisationManager();                               // Private since a singleton.
        SerialisationManager(SerialisationManager const&);    // Don't allow copy.
        void operator=(SerialisationManager const&);          // Don't allow assignment.

        // Component creation callbacks
        using ComponentCreatorFunc = std::function<void(EntityID, const std::string&)>;
        std::unordered_map<std::string, ComponentCreatorFunc> m_component_creators;

        // Component serializers
        std::unordered_map<std::string, std::shared_ptr<IComponentSerializer>> m_component_serializers;

        // Helper method for extracting Vector3D from JSON  
        Vector3D extractVector3D(const std::string& json, size_t startPos, const std::string& fieldName);

    public:
        /**
         * @brief Get the singleton instance of the SerialisationManager.
         * @return Reference to the singleton instance.
         */
        static SerialisationManager& getInstance();

        /**
         * @brief Start up the SerialisationManager.
         * @return 0 if successful, else -1.
         */
        int startUp() override;

        /**
         * @brief Shut down the SerialisationManager.
         */
        void shutDown() override;

        /**
         * @brief Load entities from a scene file.
         * @param filename The path to the scene file.
         * @return True if loading was successful, false otherwise.
         */
        bool loadScene(const std::string& filename);

        /**
         * @brief Save current entities to a scene file.
         * @param filename The path to save the scene file.
         * @return True if saving was successful, false otherwise.
         */
        bool saveScene(const std::string& filename);

        /**
         * @brief Register a component creator function.
         * @param componentName The name of the component type as it appears in scene files.
         * @param creatorFunc The function to call to create the component.
         */
        void registerComponentCreator(const std::string& componentName, ComponentCreatorFunc creatorFunc);

        /**
         * @brief Register a component serializer.
         * @param componentName The name of the component type.
         * @param serializer The serializer for this component type.
         */
        void registerComponentSerializer(const std::string& componentName, std::shared_ptr<IComponentSerializer> serializer);

        // Helper methods for parsing
        bool parseJsonFile(const std::string& filename, std::string& jsonContent);
        bool parseComponents(EntityID entityId, const std::string& componentData);

        // Helper methods for JSON parsing
        static std::string extractSection(const std::string& json, const std::string& sectionName);
        static std::string extractQuotedValue(const std::string& json, const std::string& fieldName);
        static std::vector<std::string> splitJsonArray(const std::string& jsonArray);
        static void parseKeyMappings(const std::string& keyMappingsJson, InputComponent* input);
        static void parseMouseMappings(const std::string& mouseMappingsJson, InputComponent* input);
        static std::vector<float> parseFloatArray(const std::string& arrayJson);
        static std::string extractObjectValue(const std::string& json, const std::string& fieldName);

        // Indentation helper for pretty JSON output
        std::string getIndent(int level) const;
    };

} // end of namespace gam300
#endif // __SERIALISATION_MANAGER_H__