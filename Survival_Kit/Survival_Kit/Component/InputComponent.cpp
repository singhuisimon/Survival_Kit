/**
 * @file InputComponent.cpp
 * @brief Implementation of the Input Component for the Entity Component System.
 * @details Contains implementations for all member functions declared in InputComponent.h.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#include "../Component/InputComponent.h"
#include "../Manager/InputManager.h"
#include "../Manager/LogManager.h"

namespace gam300 {

    // Constructor
    InputComponent::InputComponent() : m_is_active(true) {
        // Nothing to initialize
    }

    // Initialize the component
    void InputComponent::init(EntityID entity_id) {
        m_owner_id = entity_id;
        LM.writeLog("InputComponent::init() - Input component initialized for entity %d", entity_id);
    }

    // Update the component
    void InputComponent::update(float /*dt*/) {
        // Skip if not active
        if (!m_is_active) return;

        // Process all registered actions
        for (const auto& action_pair : m_actions) {
            const InputAction& action = action_pair.second;

            // Handle different action types
            switch (action.type) {
            case InputActionType::PRESS:
                // For key press actions, check if the key was just pressed
                if (action.input_key < GLFW_MOUSE_BUTTON_1) {
                    // It's a keyboard key
                    if (IM.isKeyJustPressed(action.input_key) && action.callback) {
                        action.callback();
                    }
                }
                else {
                    // It's a mouse button
                    if (IM.isMouseButtonJustPressed(action.input_key - GLFW_MOUSE_BUTTON_1) && action.callback) {
                        action.callback();
                    }
                }
                break;

            case InputActionType::RELEASE:
                // For key release actions, check if the key was just released
                if (action.input_key < GLFW_MOUSE_BUTTON_1) {
                    // It's a keyboard key
                    if (IM.isKeyJustReleased(action.input_key) && action.callback) {
                        action.callback();
                    }
                }
                else {
                    // It's a mouse button
                    if (IM.isMouseButtonJustReleased(action.input_key - GLFW_MOUSE_BUTTON_1) && action.callback) {
                        action.callback();
                    }
                }
                break;

            case InputActionType::REPEAT:
                // For key repeat actions, check if the key is pressed
                if (action.input_key < GLFW_MOUSE_BUTTON_1) {
                    // It's a keyboard key
                    if (IM.isKeyPressed(action.input_key) && action.callback) {
                        action.callback();
                    }
                }
                else {
                    // It's a mouse button
                    if (IM.isMouseButtonPressed(action.input_key - GLFW_MOUSE_BUTTON_1) && action.callback) {
                        action.callback();
                    }
                }
                break;

            case InputActionType::AXIS:
                // For axis actions, we handle these separately
                // No direct implementation here as they require special handling
                break;
            }
        }
    }

    // Map a key press action
    void InputComponent::mapKeyPress(const std::string& name, int key, std::function<void(void)> callback) {
        InputAction action;
        action.name = name;
        action.input_key = key;
        action.type = InputActionType::PRESS;
        action.callback = callback;

        m_actions[name] = action;
    }

    // Map a key release action
    void InputComponent::mapKeyRelease(const std::string& name, int key, std::function<void(void)> callback) {
        InputAction action;
        action.name = name;
        action.input_key = key;
        action.type = InputActionType::RELEASE;
        action.callback = callback;

        m_actions[name] = action;
    }

    // Map a key repeat action
    void InputComponent::mapKeyRepeat(const std::string& name, int key, std::function<void(void)> callback) {
        InputAction action;
        action.name = name;
        action.input_key = key;
        action.type = InputActionType::REPEAT;
        action.callback = callback;

        m_actions[name] = action;
    }

    // Map a mouse button press action
    void InputComponent::mapMousePress(const std::string& name, int button, std::function<void(void)> callback) {
        InputAction action;
        action.name = name;
        action.input_key = GLFW_MOUSE_BUTTON_1 + button; // Offset to distinguish from keyboard keys
        action.type = InputActionType::PRESS;
        action.callback = callback;

        m_actions[name] = action;
    }

    // Map a mouse button release action
    void InputComponent::mapMouseRelease(const std::string& name, int button, std::function<void(void)> callback) {
        InputAction action;
        action.name = name;
        action.input_key = GLFW_MOUSE_BUTTON_1 + button; // Offset to distinguish from keyboard keys
        action.type = InputActionType::RELEASE;
        action.callback = callback;

        m_actions[name] = action;
    }

    // Map mouse movement
    void InputComponent::mapMouseMovement(const std::string& name, std::function<void(float, float)> callback) {
        // Mark parameters as unused to avoid compiler warnings
        (void)name;      // Explicitly tell compiler that this parameter is intentionally unused
        (void)callback;  // Also mark callback as unused since it's not being used yet

        // For mouse movement, we use a special axis handler
        // This would be implemented differently in a complete system
        // For now, we just log that this functionality isn't fully implemented
        LM.writeLog("InputComponent::mapMouseMovement() - Mouse movement mapping not fully implemented yet");
    }

    // Remove an action mapping
    void InputComponent::unmapAction(const std::string& name) {
        auto it = m_actions.find(name);
        if (it != m_actions.end()) {
            m_actions.erase(it);
        }
    }

    // Set active state
    void InputComponent::setActive(bool active) {
        m_is_active = active;
    }

    // Get active state
    bool InputComponent::isActive() const {
        return m_is_active;
    }

} // namespace gam300