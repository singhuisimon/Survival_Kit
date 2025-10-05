/**
 * @file AIComponent.h
 * @brief AI Component for entities with artificial intelligence behavior.
 * @details Provides the base component for entities that require AI processing.
 * @author Simon Chan
 * @date October 5, 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once
#ifndef __AI_COMPONENT_H__
#define __AI_COMPONENT_H__

#include "../Component/Component.h"

namespace gam300 {

    /**
     * @brief Component for entities with AI behavior.
     * @details Marks entities as AI-controlled and stores AI-related state.
     */
    class AIComponent : public Component {
    public:
        /**
         * @brief Constructor for AIComponent.
         */
        AIComponent();

        /**
         * @brief Destructor for AIComponent.
         */
        ~AIComponent() override = default;

        /**
         * @brief Initialize the component.
         * @param entity_id The ID of the entity this component is attached to.
         */
        void init(EntityID entity_id) override;

        /**
         * @brief Update the component's state.
         * @param dt Delta time since last update.
         */
        void update(float dt) override;

        /**
         * @brief Check if AI is currently active.
         * @return True if AI is active, false otherwise.
         */
        bool is_active() const { return m_is_active; }

        /**
         * @brief Set the active state of the AI.
         * @param active Whether the AI should be active.
         */
        void set_active(bool active) { m_is_active = active; }

    private:
        bool m_is_active;  ///< Whether the AI is currently active
    };

} // namespace gam300

#endif // __AI_COMPONENT_H__