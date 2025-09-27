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
#ifndef __RIGIDBODY_H__
#define __RIGIDBODY_H__

#include "../Component/Component.h"
#include "../Component/Transform3D.h"
#include "../Utility/Vector3D.h"

#include "Jolt/Jolt.h"
#include "Jolt/RegisterTypes.h"
#include "Jolt/Core/Factory.h"
#include "Jolt/Core/TempAllocator.h"
#include "Jolt/Core/JobSystemThreadPool.h"
#include "Jolt/Physics/PhysicsSettings.h"
#include "Jolt/Physics/PhysicsSystem.h"
#include "Jolt/Physics/Body/BodyID.h"

namespace gam300 {

    enum class BodyType
    {
        STATIC, 
        KINEMATIC,
        DYNAMIC

    };

    class RigidBody : public Component {
    private:

        JPH::BodyID m_bodyID;           // Jolt body ID
        JPH::Body* m_body = nullptr;    // Direct reference
        BodyType m_bodyType;
        bool m_gravity;

    public:
        
        RigidBody();
        RigidBody(JPH::BodyID bodyID, JPH::Body* body,
            BodyType bodyType = BodyType::STATIC,
            const bool& gravity = true);

        RigidBody(BodyType bodyType)
            : m_bodyID(JPH::BodyID()), m_body(nullptr),
              m_bodyType(bodyType), m_gravity(true)
        {}


            // Explicit move semantics
        RigidBody(RigidBody&&) = default;
        RigidBody& operator=(RigidBody&&) = default;

        // Optional copy semantics (if ECS requires)
        RigidBody(const RigidBody&) = default;
        RigidBody& operator=(const RigidBody&) = default;

        void init(EntityID entity_id) override;

        void update(float dt) override;

        const bool& getGravity() const { return m_gravity; }
        void setType(BodyType type) { m_bodyType = type; }
        void setGravity(bool gravity) { m_gravity = gravity; }

        const float& getMass() const { return m_body ? 1.0f / m_body->GetMotionProperties()->GetInverseMass() : 0.0f; }
        const float& getInverseMass() const { return m_body ? m_body->GetMotionProperties()->GetInverseMass() : 0.0f; }
        Vector3D getLinearVelocity() const { return m_body ? convert(m_body->GetLinearVelocity()) : Vector3D(); }
        Vector3D getAngularVelocity() const { return m_body ? convert(m_body->GetAngularVelocity()) : Vector3D(); }
        const float& getLinearDamp() const { return m_body ? m_body->GetMotionProperties()->GetLinearDamping() : 0.0f; ; }
        const float& getAngularDamp() const { return m_body ? m_body->GetMotionProperties()->GetAngularDamping() : 0.0f; ; }
       
        void setLinearVelocity(const Vector3D& linearVelocity) { if (m_body) m_body->SetLinearVelocity(convert(linearVelocity)); }
        void setAngularVelocity(const Vector3D& angularVelocity) { if (m_body) m_body->SetAngularVelocity(convert(angularVelocity)); }
        void setLinearDamp(float linearDamp) { if (m_body) m_body->GetMotionProperties()->SetLinearDamping(linearDamp); }
        void setAngularDamp(float angularDamp) { if (m_body) m_body->GetMotionProperties()->SetAngularDamping(angularDamp); }

        void applyForce(const Vector3D& force);
        void applyTorque(const Vector3D& torque);
        void applyImpulse(const Vector3D& impulse);

        // add to check bodyType 
        bool isStatic() const { return m_bodyType == BodyType::STATIC; }
        bool isKinematic() const { return m_bodyType == BodyType::KINEMATIC; }
        bool isDynamic() const { return m_bodyType == BodyType::DYNAMIC; }

        // to get the type of the Rigid Body - STATIC, KINEMATIC, DYNAMIC
        const BodyType getRigidBodyType() { return m_bodyType; }

        // to return the enum type to string for serialization
        static BodyType stringToBodyType(const std::string& str);

        // convert back from string to enum for serialization
        static std::string bodyTypeToString(BodyType type);

        // set the rigid body type (use in imgui)
        void setRigidBodyType(BodyType type) { m_bodyType = type; }

        Vector3D getPosition() const {
            return m_body ? convert(m_body->GetPosition()) : Vector3D();
        }

        void setPosition(const Vector3D& pos, JPH::PhysicsSystem& system)
        {
            JPH::BodyInterface& body_interface = system.GetBodyInterface();
            body_interface.SetPosition(m_bodyID, convert(pos), JPH::EActivation::Activate);
        }

        static JPH::Vec3 convert(const Vector3D& v) { return JPH::Vec3(v.x, v.y, v.z); }
        static Vector3D convert(const JPH::Vec3& v) { return Vector3D(v.GetX(), v.GetY(), v.GetZ()); }
    };

} // namespace gam300

#endif // __RIGIDBODY_H__