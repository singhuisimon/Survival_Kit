#pragma once

#ifndef __TRANSFORM_SYSTEM_H__
#define __TRANSFORM_SYSTEM_H__

#include "System.h"
#include "../Component/TransformComponent.h"

namespace gam300{

	class TransformSystem : public ComponentSystem<TransformComponent> {

	public:
        /**
         * @brief Constructor for TransformSystem.
         */
        TransformSystem();

        /**
         * @brief Initialize the system.
         * @param system_manager Reference to the system manager.
         * @return True if initialization was successful, false otherwise.
         */
        bool init(SystemManager& system_manager) override;

        /**
         * @brief Update the system, processing all relevant entities.
         * @param dt Delta time since the last update.
         */
        void update(float dt) override;

        /**
         * @brief Clean up the system when shutting down.
         */
        void shutdown() override;

        /**
         * @brief Process a specific entity with an InputComponent.
         * @param entity_id The ID of the entity to process.
         */
        void process_entity(EntityID entity_id) override;
	};


}


#endif // !__TRANSFORM_SYSTEM_H__
