/*****************************************************************************/
/*!
\file       RigidBody.h
\author     Low Yue Jun (yuejun.low)
\par        email: yuejun.low@digipen.edu
\date       2025/10/03
\brief      Rigid body component with linear + angular forces (mask-based).
            Inherits Component. Provides full getters/setters. Linear motion
            writes position in Integrate(); angular state is updated here,
            but rotation write-back is left to the caller/system.

            (C) 2025 DigiPen Institute of Technology.
            Reproduction or disclosure of this file or its contents without the
            prior written consent of DigiPen Institute of Technology is prohibited.
*/
/*****************************************************************************/
#pragma once
#ifndef RigidBody_H
#define RigidBody_H

#include "../Component/Component.h"
#include "../Utility/Vector3D.h"
#include "../Manager/ForceManager.h"
#include "../Manager/TorqueManager.h"

namespace gam300 {
    class Transform3D;

    class RigidBody : public Component {
    public:
        /**************************************************************************
         * @brief
         * Constructs a rigid body with optional initial linear and angular state.
         *
         * @param mass
         * Mass in kilograms (<= 0 makes the body static).
         * @param velocity
         * Initial linear velocity (m/s).
         * @param acceleration
         * Initial linear acceleration (m/s^2).
         * @param inertiaDiag
         * Local-space diagonal inertia tensor (x,y,z).
         * @param angularVelocity
         * Initial angular velocity (rad/s).
         * @param forceMask
         * Bitmask gate for linear forces.
         * @param torqueMask
         * Bitmask gate for torques.
         * @param layer
         * Collision/logic layer.
         **************************************************************************/
        RigidBody(
            float              mass = 1.0f,
            const Vector3D &velocity = Vector3D{ 0.0f, 0.0f, 0.0f },
            const Vector3D &acceleration = Vector3D{ 0.0f, 0.0f, 0.0f },
            const Vector3D &inertiaDiag = Vector3D{ 0.0f, 0.0f, 0.0f },
            const Vector3D &angularVelocity = Vector3D{ 0.0f, 0.0f, 0.0f },
            unsigned           forceMask = 0xFFFFFFFFu,
            unsigned           torqueMask = 0xFFFFFFFFu,
            int                layer = 0
        );

        // ---- Component interface ----

        /**************************************************************************
         * @brief
         * Associates this component instance with an entity.
         *
         * @param entity_id
         * Target entity identifier.
         **************************************************************************/
        void init(EntityID entity_id) override;

        /**************************************************************************
         * @brief
         * Per-frame maintenance for the component (if needed).
         *
         * @param dt
         * Delta time in seconds.
         **************************************************************************/
        void update(float dt) override;

        // ---- Integration entry point ----

        /**************************************************************************
         * @brief
         * Performs a semi-implicit Euler step for linear motion and updates
         * angular state (caller is responsible for writing rotation to Transform).
         *
         * @param tr
         * Transform to write updated position into.
         * @param dt
         * Delta time in seconds.
         **************************************************************************/
        void Integrate(Transform3D &tr, float dt);

        // ---- Getters / Setters (linear) ----

        /**************************************************************************
         * @brief
         * Returns mass in kilograms.
         *
         * @return
         * Mass value.
         **************************************************************************/
        float getMass() const {
            return m_mass;
        }

        /**************************************************************************
         * @brief
         * Returns inverse mass (0.0f for static bodies).
         *
         * @return
         * Inverse mass.
         **************************************************************************/
        float getInvMass() const {
            return m_invMass;
        }

        /**************************************************************************
         * @brief
         * Returns current linear velocity.
         *
         * @return
         * Velocity vector (m/s).
         **************************************************************************/
        const Vector3D &getVelocity() const {
            return m_velocity;
        }

        /**************************************************************************
         * @brief
         * Returns current linear acceleration.
         *
         * @return
         * Acceleration vector (m/s^2).
         **************************************************************************/
        const Vector3D &getAcceleration() const {
            return m_acceleration;
        }

        /**************************************************************************
         * @brief
         * Returns active linear force mask.
         *
         * @return
         * Bitmask of enabled linear forces.
         **************************************************************************/
        unsigned getForceMask() const {
            return m_forceMask;
        }

        /**************************************************************************
         * @brief
         * Returns the collision/logic layer.
         *
         * @return
         * Layer index.
         **************************************************************************/
        int getLayer() const {
            return m_layer;
        }

        /**************************************************************************
         * @brief
         * Returns the linear ForceManager (mutable).
         *
         * @return
         * Reference to ForceManager.
         **************************************************************************/
        ForceManager &getForceManager() {
            return m_forceMgr;
        }

        /**************************************************************************
         * @brief
         * Returns the linear ForceManager (const).
         *
         * @return
         * Const reference to ForceManager.
         **************************************************************************/
        const ForceManager &getForceManager() const {
            return m_forceMgr;
        }

        /**************************************************************************
         * @brief
         * Sets mass (<= 0 makes the body static). Also updates inverse mass.
         *
         * @param m
         * Mass in kilograms.
         **************************************************************************/
        void setMass(float m);

        /**************************************************************************
         * @brief
         * Sets linear velocity.
         *
         * @param v
         * New velocity (m/s).
         **************************************************************************/
        void setVelocity(const Vector3D &v) {
            m_velocity = v;
        }

        /**************************************************************************
         * @brief
         * Sets linear acceleration.
         *
         * @param a
         * New acceleration (m/s^2).
         **************************************************************************/
        void setAcceleration(const Vector3D &a) {
            m_acceleration = a;
        }

        /**************************************************************************
         * @brief
         * Sets the linear force mask.
         *
         * @param mask
         * Bitmask for enabled linear forces.
         **************************************************************************/
        void setForceMask(unsigned mask) {
            m_forceMask = mask;
        }

        /**************************************************************************
         * @brief
         * Sets the collision/logic layer.
         *
         * @param layer
         * New layer index.
         **************************************************************************/
        void setLayer(int layer) {
            m_layer = layer;
        }

        // ---- Getters / Setters (angular) ----

        /**************************************************************************
         * @brief
         * Returns local-space diagonal inertia.
         *
         * @return
         * Inertia diagonal (x,y,z).
         **************************************************************************/
        const Vector3D &getInertiaDiagonal() const {
            return m_inertiaDiag;
        }

        /**************************************************************************
         * @brief
         * Returns inverse of the local-space diagonal inertia.
         *
         * @return
         * Inverse inertia diagonal (x,y,z).
         **************************************************************************/
        const Vector3D &getInvInertiaDiagonal() const {
            return m_invInertiaDiag;
        }

        /**************************************************************************
         * @brief
         * Returns angular velocity.
         *
         * @return
         * Angular velocity (rad/s).
         **************************************************************************/
        const Vector3D &getAngularVelocity() const {
            return m_angularVelocity;
        }

        /**************************************************************************
         * @brief
         * Returns angular acceleration.
         *
         * @return
         * Angular acceleration (rad/s^2).
         **************************************************************************/
        const Vector3D &getAngularAcceleration() const {
            return m_angularAcceleration;
        }

        /**************************************************************************
         * @brief
         * Returns active torque mask.
         *
         * @return
         * Bitmask of enabled torques.
         **************************************************************************/
        unsigned getTorqueMask() const {
            return m_torqueMask;
        }

        /**************************************************************************
         * @brief
         * Returns the TorqueManager (mutable).
         *
         * @return
         * Reference to TorqueManager.
         **************************************************************************/
        TorqueManager &getTorqueManager() {
            return m_torqueMgr;
        }

        /**************************************************************************
         * @brief
         * Returns the TorqueManager (const).
         *
         * @return
         * Const reference to TorqueManager.
         **************************************************************************/
        const TorqueManager &getTorqueManager() const {
            return m_torqueMgr;
        }

        /**************************************************************************
         * @brief
         * Sets local-space diagonal inertia and updates its inverse.
         *
         * @param I
         * Inertia diagonal (x,y,z).
         **************************************************************************/
        void setInertiaDiagonal(const Vector3D &I);

        /**************************************************************************
         * @brief
         * Sets angular velocity.
         *
         * @param w
         * Angular velocity (rad/s).
         **************************************************************************/
        void setAngularVelocity(const Vector3D &w) {
            m_angularVelocity = w;
        }

        /**************************************************************************
         * @brief
         * Sets angular acceleration.
         *
         * @param a
         * Angular acceleration (rad/s^2).
         **************************************************************************/
        void setAngularAcceleration(const Vector3D &a) {
            m_angularAcceleration = a;
        }

        /**************************************************************************
         * @brief
         * Sets the torque mask.
         *
         * @param mask
         * Bitmask for enabled torques.
         **************************************************************************/
        void setTorqueMask(unsigned mask) {
            m_torqueMask = mask;
        }

        // ---- Convenience ----

        /**************************************************************************
         * @brief
         * Returns true if the body is static (inverse mass == 0).
         *
         * @return
         * Boolean indicating static state.
         **************************************************************************/
        bool isStatic() const {
            return m_invMass <= 0.0f;
        }

        /**************************************************************************
         * @brief
         * Returns the angular displacement (rad) over dt for external rotation write-back.
         *
         * @param dt
         * Delta time in seconds.
         *
         * @return
         * Angular delta vector (rad).
         **************************************************************************/
        Vector3D GetAngularDelta(float dt) const {
            return Vector3D{ m_angularVelocity.x * dt,
                             m_angularVelocity.y * dt,
                             m_angularVelocity.z * dt };
        }

    private:
        static inline float inv_or_zero(float x) {
            return (x > 0.0f) ? (1.0f / x) : 0.0f;
        }

        // linear
        float       m_mass{};
        float       m_invMass{};
        Vector3D    m_velocity{};
        Vector3D    m_acceleration{};
        unsigned    m_forceMask{};
        int         m_layer{};
        ForceManager m_forceMgr{};

        // angular
        Vector3D      m_inertiaDiag{};
        Vector3D      m_invInertiaDiag{};
        Vector3D      m_angularVelocity{};
        Vector3D      m_angularAcceleration{};
        unsigned      m_torqueMask{};
        TorqueManager m_torqueMgr{};
    };

} // namespace gam300

#endif // RigidBody_H
