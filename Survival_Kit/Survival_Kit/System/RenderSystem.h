#pragma once

#include <unordered_map>

#include "../System/System.h"
#include "../Component/Transform3D.h"
#include "../Component/MeshComponent.h"
#include "../Component/RenderComponent.h"
#include "../Utility/Constant.h"
#include "../Graphics/Renderer.h"

namespace gam300 {

	class RenderSystem : public ComponentSystem<Transform3D, MeshComponent> {

	public:
        /**
        * @brief Constructor for RenderSystem.
        */
        RenderSystem();

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

    private:
        Camera3D              m_camera;
        Light                 m_light;

        std::vector<DrawItem> m_draw_list; 
	};

}