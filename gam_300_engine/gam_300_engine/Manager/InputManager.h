/**
 * @file InputManager.h
 * @brief Declaration of the Input Manager for the game engine.
 * @details Manages keyboard and mouse input for the game.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once
#ifndef __INPUT_MANAGER_H__
#define __INPUT_MANAGER_H__

#include "Manager.h"
#include <GLFW/glfw3.h>
#include <unordered_map>
#include <array>
#include <vector>

 // Two-letter acronym for easier access to manager.
#define IM gam300::InputManager::getInstance()

namespace gam300 {

    // Maximum number of mouse buttons to track
    constexpr int MAX_MOUSE_BUTTONS = 8;

    // Key and button states
    enum class InputState {
        RELEASED = 0,    // Not pressed
        PRESSED,         // Currently pressed
        JUST_PRESSED,    // Pressed this frame
        JUST_RELEASED    // Released this frame
    };

    class InputManager : public Manager {
    private:
        InputManager();                      // Private since a singleton.
        InputManager(InputManager const&);   // Don't allow copy.
        void operator=(InputManager const&); // Don't allow assignment.

        // Window reference for input
        GLFWwindow* m_window;

        // Key state tracking
        std::unordered_map<int, InputState> m_key_states;
        std::unordered_map<int, InputState> m_prev_key_states;

        // Mouse button state tracking
        std::array<InputState, MAX_MOUSE_BUTTONS> m_mouse_button_states;
        std::array<InputState, MAX_MOUSE_BUTTONS> m_prev_mouse_button_states;

        // Mouse position
        double m_mouse_x;
        double m_mouse_y;
        double m_prev_mouse_x;
        double m_prev_mouse_y;

        // Mouse scroll offset
        double m_scroll_x_offset;
        double m_scroll_y_offset;

        // Callback storage to avoid loss during static function callbacks
        static InputManager* s_instance;

        // Static callback functions for GLFW
        static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
        static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
        static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

    public:
        /**
         * @brief Get the singleton instance of the InputManager.
         * @return Reference to the singleton instance.
         */
        static InputManager& getInstance();

        /**
         * @brief Start up the InputManager.
         * @return 0 if successful, else -1.
         */
        int startUp() override;

        /**
         * @brief Shut down the InputManager.
         */
        void shutDown() override;

        /**
         * @brief Set the target window for input handling
         * @param window Pointer to the GLFW window
         */
        void setWindow(GLFWwindow* window);

        /**
         * @brief Update input states, should be called once per frame.
         */
        void update();

        /**
         * @brief Check if a key is currently pressed.
         * @param key GLFW key code
         * @return True if the key is currently pressed, false otherwise.
         */
        bool isKeyPressed(int key) const;

        /**
         * @brief Check if a key was just pressed this frame.
         * @param key GLFW key code
         * @return True if the key was just pressed, false otherwise.
         */
        bool isKeyJustPressed(int key) const;

        /**
         * @brief Check if a key was just released this frame.
         * @param key GLFW key code
         * @return True if the key was just released, false otherwise.
         */
        bool isKeyJustReleased(int key) const;

        /**
         * @brief Check if a mouse button is currently pressed.
         * @param button GLFW mouse button code
         * @return True if the button is currently pressed, false otherwise.
         */
        bool isMouseButtonPressed(int button) const;

        /**
         * @brief Check if a mouse button was just pressed this frame.
         * @param button GLFW mouse button code
         * @return True if the button was just pressed, false otherwise.
         */
        bool isMouseButtonJustPressed(int button) const;

        /**
         * @brief Check if a mouse button was just released this frame.
         * @param button GLFW mouse button code
         * @return True if the button was just released, false otherwise.
         */
        bool isMouseButtonJustReleased(int button) const;

        /**
         * @brief Get the current mouse X position.
         * @return Current mouse X position in screen coordinates.
         */
        double getMouseX() const;

        /**
         * @brief Get the current mouse Y position.
         * @return Current mouse Y position in screen coordinates.
         */
        double getMouseY() const;

        /**
         * @brief Get the mouse movement delta X since last frame.
         * @return Mouse X movement delta.
         */
        double getMouseDeltaX() const;

        /**
         * @brief Get the mouse movement delta Y since last frame.
         * @return Mouse Y movement delta.
         */
        double getMouseDeltaY() const;

        /**
         * @brief Get the mouse scroll X offset.
         * @return Mouse scroll X offset.
         */
        double getScrollX() const;

        /**
         * @brief Get the mouse scroll Y offset.
         * @return Mouse scroll Y offset.
         */
        double getScrollY() const;
    };

} // end of namespace gam300
#endif // __INPUT_MANAGER_H__