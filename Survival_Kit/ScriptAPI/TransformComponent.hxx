// TransformComponent.hxx
#pragma once
/**
 * @file TransformComponent.hxx
 * @brief Declaration of the TransformComponent value struct for entity position data.
 * @author Kuek Wei Jie
 * @date October 5, 2025
 * @details Provides a managed wrapper around native transform data, exposing
 *          X and Y position properties to C# scripts. Implemented as a value
 *          struct for efficient interop between native and managed code.
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
namespace ScriptAPI
{
    public value struct TransformComponent
    {
    public:
        property float X
        {
            float get();
            void set(float value);
        }
        property float Y
        {
            float get();
            void set(float value);
        }

    internal:
        TransformComponent(int id);

    private:
        int entityId;
    };
}