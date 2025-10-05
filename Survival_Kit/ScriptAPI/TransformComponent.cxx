/**
 * @file TransformComponent.cxx
 * @brief Implementation of TransformComponent property accessors for entity position data.
 * @author Kuek Wei Jie
 * @date October 5, 2025
 * @details Implements X and Y property getters and setters that interface with
 *          native engine component data, providing managed scripts access to
 *          entity transform positions.
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#include "TransformComponent.hxx"
#include "../ScriptCore/Application.h"
namespace ScriptAPI
{
    float TransformComponent::X::get()
    {
        return Core::Application::GetComponent(entityId)->x;
    }
    void TransformComponent::X::set(float value)
    {
        Core::Application::GetComponent(entityId)->x = value;
    }
    float TransformComponent::Y::get()
    {
        return Core::Application::GetComponent(entityId)->y;
    }
    void TransformComponent::Y::set(float value)
    {
        Core::Application::GetComponent(entityId)->y = value;
    }
    TransformComponent::TransformComponent(int id)
        : entityId{ id }
    {
    }
}