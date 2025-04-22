/**
 * @file InputKeyMappings.h
 * @brief Maps string key/button names to GLFW key/button codes.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once
#ifndef __INPUT_KEY_MAPPINGS_H__
#define __INPUT_KEY_MAPPINGS_H__

#include <GLFW/glfw3.h>
#include <string>
#include <unordered_map>

namespace gam300 {

    /**
     * @brief Convert a string key name to GLFW key code.
     * @param keyName The name of the key (e.g., "A", "SPACE", "LEFT_SHIFT")
     * @return The GLFW key code, or GLFW_KEY_UNKNOWN if not found.
     */
    const std::unordered_map<std::string, int>& getKeyNameMap() {
        static const std::unordered_map<std::string, int> keyMap = {
            // Letters
            {"A", GLFW_KEY_A}, {"B", GLFW_KEY_B}, {"C", GLFW_KEY_C}, {"D", GLFW_KEY_D},
            {"E", GLFW_KEY_E}, {"F", GLFW_KEY_F}, {"G", GLFW_KEY_G}, {"H", GLFW_KEY_H},
            {"I", GLFW_KEY_I}, {"J", GLFW_KEY_J}, {"K", GLFW_KEY_K}, {"L", GLFW_KEY_L},
            {"M", GLFW_KEY_M}, {"N", GLFW_KEY_N}, {"O", GLFW_KEY_O}, {"P", GLFW_KEY_P},
            {"Q", GLFW_KEY_Q}, {"R", GLFW_KEY_R}, {"S", GLFW_KEY_S}, {"T", GLFW_KEY_T},
            {"U", GLFW_KEY_U}, {"V", GLFW_KEY_V}, {"W", GLFW_KEY_W}, {"X", GLFW_KEY_X},
            {"Y", GLFW_KEY_Y}, {"Z", GLFW_KEY_Z},

            // Numbers
            {"0", GLFW_KEY_0}, {"1", GLFW_KEY_1}, {"2", GLFW_KEY_2}, {"3", GLFW_KEY_3},
            {"4", GLFW_KEY_4}, {"5", GLFW_KEY_5}, {"6", GLFW_KEY_6}, {"7", GLFW_KEY_7},
            {"8", GLFW_KEY_8}, {"9", GLFW_KEY_9},

            // Function keys
            {"F1", GLFW_KEY_F1}, {"F2", GLFW_KEY_F2}, {"F3", GLFW_KEY_F3}, {"F4", GLFW_KEY_F4},
            {"F5", GLFW_KEY_F5}, {"F6", GLFW_KEY_F6}, {"F7", GLFW_KEY_F7}, {"F8", GLFW_KEY_F8},
            {"F9", GLFW_KEY_F9}, {"F10", GLFW_KEY_F10}, {"F11", GLFW_KEY_F11}, {"F12", GLFW_KEY_F12},

            // Special keys
            {"SPACE", GLFW_KEY_SPACE}, {"TAB", GLFW_KEY_TAB}, {"ENTER", GLFW_KEY_ENTER},
            {"ESCAPE", GLFW_KEY_ESCAPE}, {"BACKSPACE", GLFW_KEY_BACKSPACE},

            // Arrow keys
            {"UP", GLFW_KEY_UP}, {"DOWN", GLFW_KEY_DOWN}, {"LEFT", GLFW_KEY_LEFT}, {"RIGHT", GLFW_KEY_RIGHT},

            // Modifiers
            {"LEFT_SHIFT", GLFW_KEY_LEFT_SHIFT}, {"RIGHT_SHIFT", GLFW_KEY_RIGHT_SHIFT},
            {"LEFT_CONTROL", GLFW_KEY_LEFT_CONTROL}, {"RIGHT_CONTROL", GLFW_KEY_RIGHT_CONTROL},
            {"LEFT_ALT", GLFW_KEY_LEFT_ALT}, {"RIGHT_ALT", GLFW_KEY_RIGHT_ALT}
        };

        return keyMap;
    }

    /**
     * @brief Convert a string mouse button name to GLFW mouse button code.
     * @param buttonName The name of the mouse button (e.g., "LEFT", "RIGHT", "MIDDLE")
     * @return The GLFW mouse button code, or -1 if not found.
     */
    const std::unordered_map<std::string, int>& getMouseButtonNameMap() {
        static const std::unordered_map<std::string, int> buttonMap = {
            {"LEFT", GLFW_MOUSE_BUTTON_LEFT},
            {"RIGHT", GLFW_MOUSE_BUTTON_RIGHT},
            {"MIDDLE", GLFW_MOUSE_BUTTON_MIDDLE},
            {"BUTTON_4", GLFW_MOUSE_BUTTON_4},
            {"BUTTON_5", GLFW_MOUSE_BUTTON_5},
            {"BUTTON_6", GLFW_MOUSE_BUTTON_6},
            {"BUTTON_7", GLFW_MOUSE_BUTTON_7},
            {"BUTTON_8", GLFW_MOUSE_BUTTON_8}
        };

        return buttonMap;
    }

} // end of namespace gam300
#endif // __INPUT_KEY_MAPPINGS_H__