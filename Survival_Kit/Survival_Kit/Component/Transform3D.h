/**
 * @file Transform3D.h
 * @brief Declaration of the Transform3D Component for the Entity Component System.
 * @details Handles position, rotation, and scale for 3D entities.
 * @author
 * @date
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once
#ifndef __TRANSFORM3D_H__
#define __TRANSFORM3D_H__

#include <glm-0.9.9.8/glm/gtc/quaternion.hpp>

#include "../Component/Component.h"
#include "../Utility/Vector3D.h"
#include "../Manager/SerialisationBinManager.h"

namespace gam300 {

    /**
     * @brief Component for handling 3D transformations.
     * @details Stores position, rotation, and scale information for entities in 3D space.
     */
    class Transform3D : public Component {
    private:
        Vector3D m_position;        // Current position in 3D space
        Vector3D m_prev_position;   // Previous position (useful for physics/interpolation)
        Vector3D m_rotation;        // Rotation in degrees (Euler angles: x, y, z)
        Vector3D m_scale;           // Scale factors for each axis

    public:
        /**
         * @brief Constructor for Transform3D.
         * @param position Initial position (default: origin)
         * @param rotation Initial rotation in degrees (default: no rotation)
         * @param scale Initial scale (default: unit scale)
         */
        Transform3D(const Vector3D& position = Vector3D::ZERO,
            const Vector3D& rotation = Vector3D::ZERO,
            const Vector3D& scale = Vector3D::ONE);

        /**
         * @brief Initialize the component after creation.
         * @param entity_id The ID of the entity this component is attached to.
         */
        void init(EntityID entity_id) override;

        /**
         * @brief Update the component state.
         * @param dt Delta time in seconds.
         */
        void update(float dt) override;

        // Position methods
        /**
         * @brief Get the current position.
         * @return Current position vector.
         */
        const Vector3D& getPosition() const { return m_position; }

        /**
         * @brief Set the current position.
         * @param position New position vector.
         */
        void setPosition(const Vector3D& position);

        /**
         * @brief Get the previous position.
         * @return Previous position vector.
         */
        const Vector3D& getPrevPosition() const { return m_prev_position; }

        // Rotation methods
        /**
         * @brief Get the current rotation in degrees.
         * @return Current rotation vector (Euler angles).
         */
        const Vector3D& getRotation() const { return m_rotation; }

        /**
         * @brief Set the current rotation in degrees.
         * @param rotation New rotation vector (Euler angles).
         */
        void setRotation(const Vector3D& rotation) { m_rotation = rotation; }

        /**
         * @brief Rotate by the given angles in degrees.
         * @param deltaRotation Rotation to add to current rotation.
         */
        void rotate(const Vector3D& deltaRotation) { m_rotation += deltaRotation; }

        // Scale methods
        /**
         * @brief Get the current scale.
         * @return Current scale vector.
         */
        const Vector3D& getScale() const { return m_scale; }

        /**
         * @brief Set the current scale.
         * @param scale New scale vector.
         */
        void setScale(const Vector3D& scale) { m_scale = scale; }

        /**
         * @brief Set uniform scale for all axes.
         * @param uniformScale Scale value to apply to all axes.
         */
        void setUniformScale(float uniformScale) { m_scale = Vector3D(uniformScale, uniformScale, uniformScale); }

        // Translation methods
        /**
         * @brief Translate (move) by the given offset.
         * @param translation Vector to add to current position.
         */
        void translate(const Vector3D& translation);

        /**
         * @brief Get the transformation matrix for this transform.
         * @return 4x4 transformation matrix as a flat array (column-major order).
         * @note This is a simplified version - in a real engine you'd use a proper Matrix4x4 class.
         */
        void getTransformationMatrix(float matrix[16]) const;

        /**
         * @brief Get the 4x4 transformation matrix
         * @return glm mat4 transformation matrix 
         */
        glm::mat4 getTransformationMatrix() const;

        // Utility methods
        /**
         * @brief Get the forward direction vector based on current rotation.
         * @return Forward direction vector.
         */
        Vector3D getForward() const;

        /**
         * @brief Get the right direction vector based on current rotation.
         * @return Right direction vector.
         */
        Vector3D getRight() const;

        /**
         * @brief Get the up direction vector based on current rotation.
         * @return Up direction vector.
         */
        Vector3D getUp() const;

        friend struct reflect<::gam300::Transform3D>;
    };

} // namespace gam300

#endif // __TRANSFORM3D_H__