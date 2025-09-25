#pragma once

#include <unordered_map>

#include "../System/System.h"
#include "../Component/Transform3D.h"
#include "../Component/RenderComponent.h"
#include "../Utility/Constant.h"
#include "../Graphics/ShaderProgram.h"
#include "../Graphics/Camera.h"
#include "../Graphics/Light.h"
#include "../Graphics/Shape.h"
#include "../Graphics/Framebuffer.h"

namespace gam300 {

	class RenderSystem : public ComponentSystem<Transform3D, RenderComponent> {

	public:
        /**
        * @brief Constructor for TransformSystem.
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

        void configurePipelineState();

        void draw();

    private:

        std::unordered_map<std::string, MeshGL>  m_tmp_mesh_storage;
        std::vector<ShaderProgram>               m_shader_storage;

        Camera3D                                 t_camera3d;
        Light                                    t_light;
	};

}