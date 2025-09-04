/**
 * @file InputComponent.h
 * @brief Declaration of the Input Component for the Entity Component System.
 * @details Handles input mapping and processing for game entities.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once
#ifndef __INPUT_COMPONENT_H__
#define __INPUT_COMPONENT_H__

#include "../Component/Component.h"
#include <GLFW/glfw3.h>
#include <functional>
#include <unordered_map>
#include <string>

namespace gam300 {

    // Input action types
    enum class InputActionType {
        PRESS,          // Triggered when key/button is pressed
        RELEASE,        // Triggered when key/button is released
        REPEAT,         // Triggered continuously while key/button is held
        AXIS            // Triggered for axis-based input (mouse movement, etc.)
    };

    // Input action mapping
    struct InputAction {
        std::string name;                       // Unique name for this action
        int input_key;                          // GLFW key/button code
        InputActionType type;                   // Type of action
        std::function<void(void)> callback;     // Function to call for PRESS/RELEASE/REPEAT
        std::function<void(float)> axis_callback; // Function to call for AXIS inputs with value
    };

    /**
     * @brief Component for handling entity-specific input mappings.
     * @details Maps input events to entity actions within the ECS.
     */
    class InputComponent : public Component {
    private:
        std::unordered_map<std::string, InputAction> m_actions;  // Input action mappings
        bool m_is_active;                                        // Whether this component processes input

    public:
        /**
         * @brief Constructor for InputComponent.
         */
        InputComponent();

         /**
         * @brief Get a copy of all the action mappings for serialization
         */
        const std::unordered_map<std::string, InputAction>& getActionMappings() const {
            return m_actions;
        }

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

        /**
         * @brief Map a key press action to a callback function.
         * @param name Unique identifier for this action.
         * @param key GLFW key code.
         * @param callback Function to call when the key is pressed.
         */
        void mapKeyPress(const std::string& name, int key, std::function<void(void)> callback);

        /**
         * @brief Map a key release action to a callback function.
         * @param name Unique identifier for this action.
         * @param key GLFW key code.
         * @param callback Function to call when the key is released.
         */
        void mapKeyRelease(const std::string& name, int key, std::function<void(void)> callback);

        /**
         * @brief Map a key repeat action to a callback function.
         * @param name Unique identifier for this action.
         * @param key GLFW key code.
         * @param callback Function to call while the key is held down.
         */
        void mapKeyRepeat(const std::string& name, int key, std::function<void(void)> callback);

        /**
         * @brief Map a mouse button press action to a callback function.
         * @param name Unique identifier for this action.
         * @param button GLFW mouse button code.
         * @param callback Function to call when the button is pressed.
         */
        void mapMousePress(const std::string& name, int button, std::function<void(void)> callback);

        /**
         * @brief Map a mouse button release action to a callback function.
         * @param name Unique identifier for this action.
         * @param button GLFW mouse button code.
         * @param callback Function to call when the button is released.
         */
        void mapMouseRelease(const std::string& name, int button, std::function<void(void)> callback);

        /**
         * @brief Map mouse movement to a callback function.
         * @param name Unique identifier for this action.
         * @param callback Function to call with the mouse delta.
         */
        void mapMouseMovement(const std::string& name, std::function<void(float, float)> callback);

        /**
         * @brief Remove an action mapping.
         * @param name The name of the action to remove.
         */
        void unmapAction(const std::string& name);

        /**
         * @brief Set whether this component is active.
         * @param active New active state.
         */
        void setActive(bool active);

        /**
         * @brief Check if this component is active.
         * @return True if active, false otherwise.
         */
        bool isActive() const;
    };

} // namespace gam300

#endif // __INPUT_COMPONENT_H__