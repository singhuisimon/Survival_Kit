/**
 * @file Script.h
 * @brief Declaration of the Script Component for the Entity Component System.
 * @details Handles script attachment and execution for entities.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once
#ifndef __SCRIPT_H__
#define __SCRIPT_H__

#include "../Component/Component.h"
#include "ScriptFieldData.h"

#include <string>

namespace gam300 {

    /**
     * @brief Component for attaching scripts to entities.
     * @details Stores script name and active state for script execution systems.
     */
    class Script : public Component {
    private:
        std::string m_script_name;  // Name of the script to execute
        bool m_is_active;           // Whether the script is currently active
        std::vector<ScriptFieldData> m_fields;  // NEW: Store field metadata


    public:
        /**
         * @brief Constructor for Script component.
         * @param script_name Name of the script (default: empty)
         * @param is_active Whether the script starts active (default: true)
         */
        Script(const std::string& script_name = "", bool is_active = true);

        /**
         * @brief Initialize the component after creation.
         * @param entity_id The ID of the entity this component is attached to.
         */
        void init(EntityID entity_id) override;

        /**
         * @brief Update the component state.
         * @param dt Delta time in seconds.
         */
        void update(float dt) override;

        // Getters
        /**
         * @brief Get the script name.
         * @return The name of the script.
         */
        const std::string& getScriptName() const { return m_script_name; }

        /**
         * @brief Check if the script is active.
         * @return True if active, false otherwise.
         */
        bool isActive() const { return m_is_active; }

        // Setters
        /**
         * @brief Set the script name.
         * @param script_name The new script name.
         */
        void setScriptName(const std::string& script_name) { m_script_name = script_name; }

        /**
         * @brief Set the active state of the script.
         * @param is_active The new active state.
         */
        void setActive(bool is_active) { m_is_active = is_active; }

        /**
         * @brief Toggle the active state of the script.
         */
        void toggleActive() { m_is_active = !m_is_active; }


        const std::vector<ScriptFieldData>& getFields() const { return m_fields; }

        // NEW: Refresh field data from C# (called when script is added/reloaded)
        void refreshFields();

        // NEW: Update a field value in C#
        bool updateFieldValue(const std::string& fieldName, const std::variant<float, int, bool, std::string>& value);
    };

} // namespace gam300

#endif // __SCRIPT_H__