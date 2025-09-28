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

    RigidBody::RigidBody()
        : m_bodyID(JPH::BodyID()), // default-constructed BodyID
        m_body(nullptr),
        m_bodyType(BodyType::STATIC),
        m_gravity(true)
    {}

    RigidBody::RigidBody(JPH::BodyID bodyID, JPH::Body* body,
        BodyType bodyType, const bool& gravity)
        : m_bodyID(bodyID)
        , m_body(body)
        , m_bodyType(bodyType)
        , m_gravity(gravity)
    {}

    void RigidBody::init(EntityID entity_id) {
        m_owner_id = entity_id;
        LM.writeLog("RigidBody::init() - RigidBody component initialized for entity %d", entity_id);
    }

    void RigidBody::update(float dt) {
        (void)dt;
    }

    void RigidBody::applyForce(const Vector3D& force) {
        if(m_body) m_body->AddForce(convert(force));
    }
    void RigidBody::applyTorque(const Vector3D& torque) {
        if (m_body) m_body->AddTorque(convert(torque));
    }
    void RigidBody::applyImpulse(const Vector3D& impulse) {
        if (m_body) m_body->AddImpulse(convert(impulse));
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