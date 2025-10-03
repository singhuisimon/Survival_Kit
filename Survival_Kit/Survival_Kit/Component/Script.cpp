/**
 * @file Script.cpp
 * @brief Implementation of the Script Component for the Entity Component System.
 * @details Contains implementations for all member functions declared in Script.h.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#include "../Component/Script.h"
#include "../Manager/LogManager.h"
#include "../ScriptCore/Application.h"


namespace gam300 {

    // Constructor
    Script::Script(const std::string& script_name, bool is_active)
        : m_script_name(script_name), m_is_active(is_active) {
        // Initialize with provided values
    }

    // Initialize the component
    void Script::init(EntityID entity_id) {
        m_owner_id = entity_id;
        LM.writeLog("Script::init() - Script component initialized for entity %d with script '%s'",
            entity_id, m_script_name.c_str());
    }

    // Update the component
    void Script::update(float dt) {
        // The Script component itself doesn't need much in update
        // A ScriptSystem will handle executing the actual scripts based on this component's data
        // This update is here to maintain the Component interface

        // Mark parameter as unused to avoid compiler warning
        (void)dt;
    }

    void Script::refreshFields() {
        m_fields.clear();

        // TODO: Call into C# to get field data
        // This requires adding a function pointer in Application.h
        // For now, this is a placeholder
    }

    bool Script::updateFieldValue(const std::string& fieldName, const std::variant<float, int, bool, std::string>& value) {
        // Find the field
        for (auto& field : m_fields) {
            if (field.name == fieldName) {
                field.value = value;

                // TODO: Call into C# to update the actual field
                // This requires adding a function pointer in Application.h
                return true;
            }
        }
        return false;
    }

} // namespace gam300