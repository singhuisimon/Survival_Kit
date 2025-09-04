/**
 * @file Manager.cpp
 * @brief Implementation of the Manager base class.
 * @details Contains implementations for all member functions declared in Manager.h.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#include "Manager.h"

namespace gam300 {

    // Default constructor
    Manager::Manager() : m_type(""), m_is_started(false) {
        // Initialize members with default values
    }

    // Default destructor
    Manager::~Manager() {
        // If still running, shut down properly
        if (m_is_started) {
            shutDown();
        }
    }

    // Set type identifier of Manager
    void Manager::setType(std::string new_type) {
        m_type = new_type;
    }

    // Get type identifier of Manager
    std::string Manager::getType() const {
        return m_type;
    }

    // Startup Manager
    int Manager::startUp() {
        // Base implementation just marks as started
        // Derived classes should override with specific initialization
        m_is_started = true;
        return 0; // Return success
    }

    // Shutdown Manager
    void Manager::shutDown() {
        // Base implementation just marks as stopped
        // Derived classes should override with specific cleanup
        m_is_started = false;
    }

    // Return status of is_started
    bool Manager::isStarted() const {
        return m_is_started;
    }

} // end of namespace gam300