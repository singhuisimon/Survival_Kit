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

 //Can be replaced by JoltPhysics RigidBody - Need CMake
namespace gam300 {

    enum class BodyType
    {
        STATIC, 
        KINEMATIC,
        DYNAMIC

    };

    class RigidBody : public Component {
    private:

        BodyType m_bodyType;
        float m_mass;
        float m_inverse_mass;

        Vector3D m_linear_velocity;
        Vector3D m_force_accumulator;

        Vector3D m_angular_velocity;
        Vector3D m_torque_accumulator;

        float m_linear_damp;
        float m_angular_damp;

        bool m_gravity;
        //bool m_kinematic;


    public:
       
        
        RigidBody(BodyType bodyType = BodyType::STATIC,
            const float& mass = 1.0f,
            const Vector3D& linear_velocity = Vector3D::ZERO,
            const Vector3D& force_accumulator = Vector3D::ZERO,
            const Vector3D& angular_velocity = Vector3D::ZERO,
            const Vector3D& torque_accumulator = Vector3D::ZERO,
            const float& linear_damp = 0.99f, const float& angular_damp = 0.99f,
            const bool& gravity = true);


        void init(EntityID entity_id) override;

        void update(float dt) override;

        const float& getMass() const { return m_mass; }
        const float& getInverseMass() const { return m_inverse_mass; }
        const Vector3D& getLinearVelocity() const { return m_linear_velocity; }
        const Vector3D& getForceAccumulator() const { return m_force_accumulator; }
        const Vector3D& getAngularVelocity() const { return m_angular_velocity; }
        const Vector3D& getTorqueAccumulator() const { return m_torque_accumulator; }
        const float& getLinearDamp() const { return m_linear_damp; }
        const float& getAngularDamp() const { return m_angular_damp; }
        const bool& getGravity() const { return m_gravity; }
        //const bool& getKinematic() const { return m_kinematic; }
        void setType(BodyType type) { m_bodyType = type; }
        void setMass(const float& mass) { m_mass = mass; }
        void setInverseMass(float inverseMass) { m_inverse_mass = inverseMass; }
        void setLinearVelocity(const Vector3D& linearVelocity) { m_linear_velocity = linearVelocity; }
        void setForceAccumulator(const Vector3D& forceAccumulator) { m_force_accumulator = forceAccumulator; }
        void setAngularVelocity(const Vector3D& angularVelocity) { m_angular_velocity = angularVelocity; }
        void setTorqueAccumulator(const Vector3D& torqueAccumulator) { m_torque_accumulator = torqueAccumulator; }
        void setLinearDamp(float linearDamp) { m_linear_damp = linearDamp; }
        void setAngularDamp(float angularDamp) { m_angular_damp = angularDamp; }
        void setGravity(bool gravity) { m_gravity = gravity; }
        //void setKinematic(bool kinematic) { m_kinematic = kinematic; }

        void applyForce(const Vector3D& force);
        void applyTorque(const Vector3D& torque);
        void applyImpulse(const Vector3D& impulse);

        void clearAccumulators();

        void integrateForces(float dt);
        void integrateVelocity(Transform3D& transform, float dt);

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
    };

} // namespace gam300

#endif // __RIGIDBODY_H__