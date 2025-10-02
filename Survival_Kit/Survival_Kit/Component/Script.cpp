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

} // namespace gam300