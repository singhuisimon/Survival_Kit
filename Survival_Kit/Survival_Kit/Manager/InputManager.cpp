/**
 * @file InputManager.cpp
 * @brief Implementation of the Input Manager for the game engine.
 * @details Handles keyboard and mouse input for the game.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#include "InputManager.h"
#include "LogManager.h"

namespace gam300 {

    // Initialize static instance pointer for callback access
    InputManager* InputManager::s_instance = nullptr;

    // Constructor
    InputManager::InputManager() :
        m_window(nullptr),
        m_mouse_x(0.0),
        m_mouse_y(0.0),
        m_prev_mouse_x(0.0),
        m_prev_mouse_y(0.0),
        m_scroll_x_offset(0.0),
        m_scroll_y_offset(0.0) {

        setType("InputManager");

        // Initialize mouse button states
        for (auto& state : m_mouse_button_states) {
            state = InputState::RELEASED;
        }
        for (auto& state : m_prev_mouse_button_states) {
            state = InputState::RELEASED;
        }
    }

    // Get the singleton instance
    InputManager& InputManager::getInstance() {
        static InputManager instance;
        s_instance = &instance; // Set static instance for callbacks
        return instance;
    }

    // Start up the InputManager
    int InputManager::startUp() {
        // Call parent's startUp() first
        if (Manager::startUp())
            return -1;

        // Log startup
        LM.writeLog("InputManager::startUp() - Input Manager started successfully");

        return 0;
    }

    // Shut down the InputManager
    void InputManager::shutDown() {
        // Log shutdown
        LM.writeLog("InputManager::shutDown() - Shutting down Input Manager");

        // Reset window callbacks if we have a window
        if (m_window) {
            glfwSetKeyCallback(m_window, nullptr);
            glfwSetMouseButtonCallback(m_window, nullptr);
            glfwSetCursorPosCallback(m_window, nullptr);
            glfwSetScrollCallback(m_window, nullptr);
            m_window = nullptr;
        }

        // Clear stored states
        m_key_states.clear();
        m_prev_key_states.clear();

        // Call parent's shutDown()
        Manager::shutDown();
    }

    // Set the window to handle input for
    void InputManager::setWindow(GLFWwindow* window) {
        m_window = window;

        if (m_window) {
            // Set up GLFW callbacks
            glfwSetKeyCallback(m_window, key_callback);
            glfwSetMouseButtonCallback(m_window, mouse_button_callback);
            glfwSetCursorPosCallback(m_window, cursor_position_callback);
            glfwSetScrollCallback(m_window, scroll_callback);

            // Get initial cursor position
            glfwGetCursorPos(m_window, &m_mouse_x, &m_mouse_y);
            m_prev_mouse_x = m_mouse_x;
            m_prev_mouse_y = m_mouse_y;

            LM.writeLog("InputManager::setWindow() - Window set and callbacks registered");
        }
    }

    // Update input states, should be called once per frame
    void InputManager::update() {
        // Store previous states for transitions BEFORE processing new states
        m_prev_key_states = m_key_states;
        m_prev_mouse_button_states = m_mouse_button_states;

        // Store previous mouse position
        m_prev_mouse_x = m_mouse_x;
        m_prev_mouse_y = m_mouse_y;

        // Update mouse cursor position
        glfwGetCursorPos(m_window, &m_mouse_x, &m_mouse_y);

        // IMPORTANT: Poll events to trigger callbacks before processing state transitions
        glfwPollEvents();

        // Update key states from JUST_PRESSED to PRESSED and JUST_RELEASED to RELEASED
        for (auto& key_state : m_key_states) {
            if (key_state.second == InputState::JUST_PRESSED) {
                key_state.second = InputState::PRESSED;
            }
            else if (key_state.second == InputState::JUST_RELEASED) {
                key_state.second = InputState::RELEASED;
            }
        }

        // Update mouse button states
        for (int i = 0; i < MAX_MOUSE_BUTTONS; ++i) {
            if (m_mouse_button_states[i] == InputState::JUST_PRESSED) {
                m_mouse_button_states[i] = InputState::PRESSED;
            }
            else if (m_mouse_button_states[i] == InputState::JUST_RELEASED) {
                m_mouse_button_states[i] = InputState::RELEASED;
            }
        }
    }

    // Check if a key is currently pressed
    bool InputManager::isKeyPressed(int key) const {
        auto it = m_key_states.find(key);
        if (it != m_key_states.end()) {
            return it->second == InputState::PRESSED || it->second == InputState::JUST_PRESSED;
        }
        return false;
    }

    // Check if a key was just pressed this frame
    bool InputManager::isKeyJustPressed(int key) const {
        auto it = m_key_states.find(key);
        if (it != m_key_states.end()) {
            return it->second == InputState::JUST_PRESSED;
        }
        return false;
    }

    // Check if a key was just released this frame
    bool InputManager::isKeyJustReleased(int key) const {
        auto it = m_key_states.find(key);
        if (it != m_key_states.end()) {
            return it->second == InputState::JUST_RELEASED;
        }
        return false;
    }

    // Check if a mouse button is currently pressed
    bool InputManager::isMouseButtonPressed(int button) const {
        if (button >= 0 && button < MAX_MOUSE_BUTTONS) {
            return m_mouse_button_states[button] == InputState::PRESSED ||
                m_mouse_button_states[button] == InputState::JUST_PRESSED;
        }
        return false;
    }

    // Check if a mouse button was just pressed this frame
    bool InputManager::isMouseButtonJustPressed(int button) const {
        if (button >= 0 && button < MAX_MOUSE_BUTTONS) {
            return m_mouse_button_states[button] == InputState::JUST_PRESSED;
        }
        return false;
    }

    // Check if a mouse button was just released this frame
    bool InputManager::isMouseButtonJustReleased(int button) const {
        if (button >= 0 && button < MAX_MOUSE_BUTTONS) {
            return m_mouse_button_states[button] == InputState::JUST_RELEASED;
        }
        return false;
    }

    // Get the current mouse X position
    double InputManager::getMouseX() const {
        return m_mouse_x;
    }

    // Get the current mouse Y position
    double InputManager::getMouseY() const {
        return m_mouse_y;
    }

    // Get the mouse movement delta X since last frame
    double InputManager::getMouseDeltaX() const {
        return m_mouse_x - m_prev_mouse_x;
    }

    // Get the mouse movement delta Y since last frame
    double InputManager::getMouseDeltaY() const {
        return m_mouse_y - m_prev_mouse_y;
    }

    // Get the mouse scroll X offset
    double InputManager::getScrollX() const {
        return m_scroll_x_offset;
    }

    // Get the mouse scroll Y offset
    double InputManager::getScrollY() const {
        return m_scroll_y_offset;
    }

    // GLFW Callback for keyboard input
    void InputManager::key_callback(GLFWwindow* /*window*/, int key, int /*scancode*/, int action, int /*mods*/) {
        if (s_instance) {
            if (action == GLFW_PRESS) {
                s_instance->m_key_states[key] = InputState::JUST_PRESSED;
            }
            else if (action == GLFW_RELEASE) {
                s_instance->m_key_states[key] = InputState::JUST_RELEASED;
            }
        }
    }

    // GLFW Callback for mouse button input
    void InputManager::mouse_button_callback(GLFWwindow* /*window*/, int button, int action, int /*mods*/) {
        if (s_instance && button >= 0 && button < MAX_MOUSE_BUTTONS) {
            if (action == GLFW_PRESS) {
                s_instance->m_mouse_button_states[button] = InputState::JUST_PRESSED;
            }
            else if (action == GLFW_RELEASE) {
                s_instance->m_mouse_button_states[button] = InputState::JUST_RELEASED;
            }
        }
    }

    // GLFW Callback for cursor position
    void InputManager::cursor_position_callback(GLFWwindow* /*window*/, double xpos, double ypos) {
        if (s_instance) {
            s_instance->m_mouse_x = xpos;
            s_instance->m_mouse_y = ypos;
        }
    }

    // GLFW Callback for scroll input
    void InputManager::scroll_callback(GLFWwindow* /*window*/, double xoffset, double yoffset) {
        if (s_instance) {
            s_instance->m_scroll_x_offset = xoffset;
            s_instance->m_scroll_y_offset = yoffset;
        }
    }

} // end of namespace gam300