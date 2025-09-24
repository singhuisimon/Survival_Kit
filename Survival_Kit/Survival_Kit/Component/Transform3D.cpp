/**
 * @file Transform3D.cpp
 * @brief Implementation of the Transform3D Component for the Entity Component System.
 * @details Contains implementations for all member functions declared in Transform3D.h.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#include "../Component/Transform3D.h"
#include "../Manager/LogManager.h"
#include "../Utility/MathUtils.h"

#include <glm-0.9.9.8/glm/gtx/quaternion.hpp>

#include <cmath>

namespace gam300 {

    REFLECT_TYPE(Transform3D,
        &Transform3D::m_position,
        &Transform3D::m_prev_position,
        &Transform3D::m_rotation,
        &Transform3D::m_scale
    );

    // Constructor
    Transform3D::Transform3D(const Vector3D& position, const Vector3D& rotation, const Vector3D& scale)
        : m_position(position), m_prev_position(position), m_rotation(rotation), m_scale(scale) {
        // Initialize with provided values
    }

    // Initialize the component
    void Transform3D::init(EntityID entity_id) {
        m_owner_id = entity_id;
        LM.writeLog("Transform3D::init() - Transform3D component initialized for entity %d", entity_id);
    }

    // Update the component
    void Transform3D::update(float dt) {
        // Store previous position for physics/interpolation
        m_prev_position = m_position;

        // Transform3D doesn't need to do much in update - it's primarily a data container
        // Physics systems, rendering systems, etc. will read and modify this data

        // Mark parameter as unused to avoid compiler warning
        (void)dt;
    }

    // Set position and update previous position
    void Transform3D::setPosition(const Vector3D& position) {
        m_prev_position = m_position;
        m_position = position;
    }

    // Translate by offset
    void Transform3D::translate(const Vector3D& translation) {
        m_prev_position = m_position;
        m_position += translation;
    }

    // Get transformation matrix (simplified 4x4 matrix)
    void Transform3D::getTransformationMatrix(float matrix[16]) const {
        // Convert rotation from degrees to radians
        float rotX = MathUtils::toRadians(m_rotation.x);
        float rotY = MathUtils::toRadians(m_rotation.y);
        float rotZ = MathUtils::toRadians(m_rotation.z);

        // Calculate trigonometric values
        float cosX = std::cos(rotX), sinX = std::sin(rotX);
        float cosY = std::cos(rotY), sinY = std::sin(rotY);
        float cosZ = std::cos(rotZ), sinZ = std::sin(rotZ);

        // Create rotation matrices and combine them (ZYX order)
        // This is a simplified version - a real engine would use proper matrix classes

        // Combined rotation matrix (ZYX Euler angles)
        float r00 = cosY * cosZ;
        float r01 = cosY * sinZ;
        float r02 = -sinY;

        float r10 = sinX * sinY * cosZ - cosX * sinZ;
        float r11 = sinX * sinY * sinZ + cosX * cosZ;
        float r12 = sinX * cosY;

        float r20 = cosX * sinY * cosZ + sinX * sinZ;
        float r21 = cosX * sinY * sinZ - sinX * cosZ;
        float r22 = cosX * cosY;

        // Apply scale and create final transformation matrix (column-major order)
        matrix[0] = r00 * m_scale.x;  matrix[4] = r01 * m_scale.y;  matrix[8] = r02 * m_scale.z;  matrix[12] = m_position.x;
        matrix[1] = r10 * m_scale.x;  matrix[5] = r11 * m_scale.y;  matrix[9] = r12 * m_scale.z;  matrix[13] = m_position.y;
        matrix[2] = r20 * m_scale.x;  matrix[6] = r21 * m_scale.y;  matrix[10] = r22 * m_scale.z;  matrix[14] = m_position.z;
        matrix[3] = 0.0f;             matrix[7] = 0.0f;             matrix[11] = 0.0f;             matrix[15] = 1.0f;
    }

    glm::mat4 Transform3D::getTransformationMatrix() const {

        glm::vec3 trl = static_cast<glm::vec3>(m_position);
        glm::vec3 rot = static_cast<glm::vec3>(m_rotation);
        glm::vec3 scl = static_cast<glm::vec3>(m_scale);

        glm::mat4 trl_mat = glm::translate(glm::mat4(1.0f), trl);
        glm::mat4 scl_mat = glm::scale(glm::mat4(1.0f), scl);

        auto quat_x = glm::angleAxis(glm::radians(rot.x), glm::vec3(1.0f, 0.0f, 0.0f));
        auto quat_y = glm::angleAxis(glm::radians(rot.y), glm::vec3(0.0f, 1.0f, 0.0f));
        auto quat_z = glm::angleAxis(glm::radians(rot.z), glm::vec3(0.0f, 0.0f, 1.0f));

        auto quat_final = quat_z * quat_y * quat_x;

        glm::mat4 rot_mat = glm::toMat4(quat_final);

        glm::mat4 trans_mat = trl_mat * rot_mat * scl_mat;

        return trans_mat;
    }

    // Get forward direction vector
    Vector3D Transform3D::getForward() const {
        // Forward is typically negative Z in OpenGL convention
        float rotY = MathUtils::toRadians(m_rotation.y);
        float rotX = MathUtils::toRadians(m_rotation.x);

        return Vector3D(
            std::sin(rotY) * std::cos(rotX),
            -std::sin(rotX),
            -std::cos(rotY) * std::cos(rotX)
        ).normalize();
    }

    // Get right direction vector
    Vector3D Transform3D::getRight() const {
        float rotY = MathUtils::toRadians(m_rotation.y);

        return Vector3D(
            std::cos(rotY),
            0.0f,
            std::sin(rotY)
        ).normalize();
    }

    // Get up direction vector
    Vector3D Transform3D::getUp() const {
        // Up is perpendicular to both forward and right
        return Vector3D::cross(getRight(), getForward()).normalize();
    }

} // namespace gam300