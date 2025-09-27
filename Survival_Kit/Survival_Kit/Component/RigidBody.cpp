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

#include "../Component/RigidBody.h"
#include "../Manager/LogManager.h"
#include "../Utility/MathUtils.h"

#include <cmath>

namespace gam300 {

    RigidBody::RigidBody(
        BodyType bodyType,
        const float& mass,
        const Vector3D& linear_velocity, 
        const Vector3D& force_accumulator,
        const Vector3D& angular_velocity, 
        const Vector3D& torque_accumulator,
        const float& linear_damp, 
        const float& angular_damp,
        const bool& gravity) : 
        m_bodyType(bodyType),
        m_mass(mass),
        m_linear_velocity(linear_velocity),
        m_force_accumulator(force_accumulator),
        m_angular_velocity(angular_velocity),
        m_torque_accumulator(torque_accumulator),
        m_linear_damp(linear_damp),
        m_angular_damp(angular_damp),
        m_gravity(gravity)
    {
        if (isStatic() || isKinematic()) {
            m_inverse_mass = 0.0f; 
        }
        else if (isDynamic()){
            m_inverse_mass = 1.0f / m_mass; 
        }
    }

    void RigidBody::init(EntityID entity_id) {
        m_owner_id = entity_id;
        LM.writeLog("RigidBody::init() - RigidBody component initialized for entity %d", entity_id);
    }

    void RigidBody::update(float dt) {
        (void)dt;
    }

    void RigidBody::applyForce(const Vector3D& force) {
        if (isDynamic()) {
            m_force_accumulator += force;
        }
    }
    void RigidBody::applyTorque(const Vector3D& torque) {
        if (isDynamic()) {
            m_torque_accumulator += torque;
        }
    }
    void RigidBody::applyImpulse(const Vector3D& impulse) {
        if (isDynamic()) {
            m_linear_velocity += impulse * m_inverse_mass;
        }
    }

    void RigidBody::clearAccumulators() {
        m_force_accumulator = Vector3D::ZERO;
        m_torque_accumulator = Vector3D::ZERO;
    }

    void RigidBody::integrateForces(float dt) {

        if (isStatic() || isKinematic()) return;

        if (m_gravity) {
            Vector3D gravityForce = { 0.0f, -9.81f * m_mass, 0.0f };
            m_force_accumulator += gravityForce;
        }

        // Linear acceleration
        Vector3D acceleration = m_force_accumulator * m_inverse_mass;
        m_linear_velocity += acceleration * dt;

        // Angular acceleration 
        m_angular_velocity += m_torque_accumulator * dt;

        // Apply damping
        m_linear_velocity *= m_linear_damp;
        m_angular_velocity *= m_angular_damp;
    }

    void RigidBody::integrateVelocity(Transform3D& transform, float dt) {
        if (isStatic() || isKinematic()) return;

        if (isDynamic()) {
            transform.setPosition(transform.getPosition() + (m_linear_velocity * dt));
            transform.setRotation(transform.getRotation() + (m_angular_velocity * dt));
        }
        
        //transform.setPosition(transform.getPosition() + (m_linear_velocity * dt));
        //transform.setRotation(transform.getRotation() + (m_angular_velocity * dt));
    }

    BodyType RigidBody::stringToBodyType(const std::string& str)
    {
        if (str == "STATIC") return BodyType::STATIC;
        if (str == "KINEMATIC") return BodyType::KINEMATIC;
        if (str == "DYNAMIC") return BodyType::DYNAMIC;

        return BodyType::STATIC;
    }

    std::string RigidBody::bodyTypeToString(BodyType type)
    {
        switch (type) {
        case BodyType::STATIC:    return "STATIC";
        case BodyType::KINEMATIC: return "KINEMATIC";
        case BodyType::DYNAMIC:   return "DYNAMIC";
        default:                  return "UNKNOWN";
        }
    }

} // namespace gam300