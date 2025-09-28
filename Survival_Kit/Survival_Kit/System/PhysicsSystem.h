/**
 * @file ...
 * @brief ...
 * @details ...
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once

#ifndef __PHYSICS_SYSTEM_H__
#define __PHYSICS_SYSTEM_H__


#include "../Component/Transform3D.h"
#include "../Component/RigidBody.h"
#include "../System/System.h"
#include <glm-0.9.9.8/glm/gtx/quaternion.hpp>



#include "Jolt/Jolt.h"
#include "Jolt/RegisterTypes.h"
#include "Jolt/Core/Factory.h"
#include "Jolt/Core/TempAllocator.h"
#include "Jolt/Core/JobSystemThreadPool.h"
#include "Jolt/Physics/PhysicsSettings.h"
#include "Jolt/Physics/PhysicsSystem.h"
#include "Jolt/Physics/Body/BodyID.h"


namespace gam300
{

    class SimpleBroadPhaseLayerInterface : public JPH::BroadPhaseLayerInterface {
    public:
        JPH::uint GetNumBroadPhaseLayers() const override { return 2; }
        JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer layer) const override {
            return JPH::BroadPhaseLayer(layer < 2 ? layer : 0);
        }
    };

    class SimpleObjectVsBroadPhaseLayerFilter : public JPH::ObjectVsBroadPhaseLayerFilter {
    public:
        bool ShouldCollide(JPH::ObjectLayer, JPH::BroadPhaseLayer) const override {
            return true; // Allow all collisions for now
        }
    };

    class SimpleObjectLayerPairFilter : public JPH::ObjectLayerPairFilter {
    public:
        bool ShouldCollide(JPH::ObjectLayer, JPH::ObjectLayer) const override {
            return true; // Allow all collisions for now
        }
    };

    class PhysicsSystem : public ComponentSystem< RigidBody> {

    public:
        /**
        * @brief Constructor for PhysicsSystem
        */
        PhysicsSystem();

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


        //    // helper function 
        //    //void handleKINEMATIC(Transform3D&  transform, RigidBody rigidBody);

        //    void handleDynamic(Transform3D& transform, RigidBody& rigidBody);


    private:
        //float m_dt = 0;
        JPH::PhysicsSystem* joltPhysics = nullptr;
        JPH::TempAllocatorImpl* tempAllocator = nullptr;
        JPH::JobSystemThreadPool* jobSystem = nullptr;
        SimpleBroadPhaseLayerInterface* broadPhaseLayerInterface = nullptr;
        float m_dt = 0.0f;
    };

}
#endif // !__PHYSICS_SYSTEM_H__
