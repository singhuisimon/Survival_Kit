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
#include "../Utility/ECS_Variables.h"

 // Two-letter acronym for easier access to manager.
#define SEM gam300::SerialisationManager::getInstance()

namespace gam300 {

    // Forward declarations
    class Entity;

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

        // Helper methods for parsing
        bool parseJsonFile(const std::string& filename, std::string& jsonContent);
        bool parseComponents(EntityID entityId, const std::string& componentData);

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
    };

} // end of namespace gam300
#endif // __SERIALISATION_MANAGER_H__