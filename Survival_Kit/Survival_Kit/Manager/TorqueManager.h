/*****************************************************************************/
/*!
\file       TorqueManager.h
\author     Low Yue Jun (yuejun.low)
\par        email: yuejun.low@digipen.edu
\date       2025/10/03
\brief      Torque framework:
            - Base Torque with lifetime / activation / mask gating
            - AngularDirectionalTorque and AngularDrag implementations
            - TorqueManager for accumulation, masking, and lifecycle control

            (C) 2025 DigiPen Institute of Technology.
            Reproduction or disclosure of this file or its contents without the
            prior written consent of DigiPen Institute of Technology is prohibited.
*/
/*****************************************************************************/
#pragma once
#ifndef __TORQUE_MANAGER_H__
#define __TORQUE_MANAGER_H__

#include <algorithm>
#include <vector>
#include <memory>
#include <unordered_map>
#include <string>
#include "../Utility/Vector3D.h"

namespace gam300 {
    /**************************************************************************
     * @brief
     * Base interface for angular torques with lifetime, activation, and masks.
     **************************************************************************/
    class Torque {
    public:
        /**************************************************************************
         * @brief
         * Lifetime categories for a torque.
         **************************************************************************/
        enum DURATION {
            Temp, Perm, Impulse
        };

    protected:
        DURATION durationType;
        float    lifetime;
        bool     isActive;
        float    age;
        unsigned torqueMask;

    public:
        /**************************************************************************
         * @brief
         * Constructs a torque with duration category and mask.
         *
         * @param type
         * Duration category (Temp, Perm, Impulse).
         *
         * @param mask
         * Category mask used for filtering and control.
         **************************************************************************/
        Torque(DURATION type, unsigned mask);
        virtual ~Torque() = default;

        /**************************************************************************
         * @brief
         * Returns configured lifetime in seconds (-1 for Perm).
         *
         * @return
         * Lifetime value.
         **************************************************************************/
        float GetLifetime() const;

        /**************************************************************************
         * @brief
         * Sets lifetime for Temp torques (ignored for Perm).
         *
         * @param value
         * Lifetime in seconds.
         **************************************************************************/
        void SetLifetime(float value);

        /**************************************************************************
         * @brief
         * Returns activation state.
         *
         * @return
         * True if active; otherwise false.
         **************************************************************************/
        bool GetIsActive() const;

        /**************************************************************************
         * @brief
         * Sets activation state.
         *
         * @param value
         * New activation state.
         **************************************************************************/
        void SetIsActive(bool value);

        /**************************************************************************
         * @brief
         * Returns current age in seconds.
         *
         * @return
         * Accumulated age.
         **************************************************************************/
        float GetAge() const;

        /**************************************************************************
         * @brief
         * Sets current age in seconds.
         *
         * @param value
         * New age.
         **************************************************************************/
        void SetAge(float value);

        /**************************************************************************
         * @brief
         * Returns category mask.
         *
         * @return
         * Mask bits.
         **************************************************************************/
        unsigned GetTorqueMask() const;

        /**************************************************************************
         * @brief
         * Returns duration category.
         *
         * @return
         * DURATION value.
         **************************************************************************/
        DURATION GetDurationType() const;

        /**************************************************************************
         * @brief
         * Deactivates Temp torques if age exceeds lifetime.
         *
         * @return
         * False if expired and deactivated; true otherwise.
         **************************************************************************/
        bool ValidateAge();

        /**************************************************************************
         * @brief
         * Advances age by delta time and validates lifetime for Temp torques.
         *
         * @param deltaTime
         * Time step (seconds).
         **************************************************************************/
        void Update(double deltaTime);

        /**************************************************************************
         * @brief
         * Computes torque contribution given current angular velocity.
         *
         * @param currentAngularVelocity
         * Angular velocity sample (used by velocity-dependent torques).
         *
         * @return
         * Torque vector to accumulate.
         **************************************************************************/
        virtual Vector3D CalculateTorque(const Vector3D &currentAngularVelocity) const = 0;

        /**************************************************************************
         * @brief
         * Polymorphic clone.
         *
         * @return
         * Unique pointer to a copy.
         **************************************************************************/
        virtual std::unique_ptr<Torque> Clone() const = 0;
    };

    /**************************************************************************
     * @brief
     * Constant-axis torque: tau = axis * magnitude.
     **************************************************************************/
    class AngularDirectionalTorque : public Torque {
    private:
        Vector3D unitAxis;
        float    magnitude;

    public:
        /**************************************************************************
         * @brief
         * Constructs a constant-axis torque.
         *
         * @param axisUnit
         * Axis to apply (must be unit; normalized safely internally).
         *
         * @param magnitude
         * Scalar magnitude.
         *
         * @param mask
         * Category mask.
         *
         * @param type
         * Duration category.
         **************************************************************************/
        AngularDirectionalTorque(const Vector3D &axisUnit,
                                 float           magnitude,
                                 unsigned        mask,
                                 DURATION        type);

        /**************************************************************************
         * @brief
         * Returns unit axis.
         *
         * @return
         * Unit axis vector.
         **************************************************************************/
        Vector3D GetUnitAxis() const;

        /**************************************************************************
         * @brief
         * Sets unit axis (normalized safely).
         *
         * @param axis
         * New axis vector.
         **************************************************************************/
        void SetUnitAxis(const Vector3D &axis);

        /**************************************************************************
         * @brief
         * Returns torque magnitude.
         *
         * @return
         * Scalar magnitude.
         **************************************************************************/
        float GetMagnitude() const;

        /**************************************************************************
         * @brief
         * Sets torque magnitude.
         *
         * @param value
         * New magnitude.
         **************************************************************************/
        void SetMagnitude(float value);

        /**************************************************************************
         * @brief
         * Computes torque contribution.
         *
         * @param currentAngularVelocity
         * Current angular velocity (unused by this type).
         *
         * @return
         * Torque vector.
         **************************************************************************/
        Vector3D CalculateTorque(const Vector3D &currentAngularVelocity) const override;

        /**************************************************************************
         * @brief
         * Polymorphic clone.
         *
         * @return
         * Unique pointer to a copy.
         **************************************************************************/
        std::unique_ptr<Torque> Clone() const override;
    };

    /**************************************************************************
     * @brief
     * Per-axis angular drag: tau = -C .* w (X,Y,Z coefficients).
     **************************************************************************/
    class AngularDrag : public Torque {
    private:
        float dragX;
        float dragY;
        float dragZ;

    public:
        /**************************************************************************
         * @brief
         * Constructs per-axis angular drag.
         *
         * @param cx
         * Drag coefficient for X.
         *
         * @param cy
         * Drag coefficient for Y.
         *
         * @param cz
         * Drag coefficient for Z.
         *
         * @param mask
         * Category mask.
         *
         * @param type
         * Duration category (default Perm).
         **************************************************************************/
        AngularDrag(float cx, float cy, float cz, unsigned mask,
                    DURATION type = DURATION::Perm);

        /**************************************************************************
         * @brief
         * Returns X-axis drag coefficient.
         *
         * @return
         * Coefficient value.
         **************************************************************************/
        float GetDragX() const;

        /**************************************************************************
         * @brief
         * Returns Y-axis drag coefficient.
         *
         * @return
         * Coefficient value.
         **************************************************************************/
        float GetDragY() const;

        /**************************************************************************
         * @brief
         * Returns Z-axis drag coefficient.
         *
         * @return
         * Coefficient value.
         **************************************************************************/
        float GetDragZ() const;

        /**************************************************************************
         * @brief
         * Sets X-axis drag coefficient.
         *
         * @param v
         * New coefficient.
         **************************************************************************/
        void SetDragX(float v);

        /**************************************************************************
         * @brief
         * Sets Y-axis drag coefficient.
         *
         * @param v
         * New coefficient.
         **************************************************************************/
        void SetDragY(float v);

        /**************************************************************************
         * @brief
         * Sets Z-axis drag coefficient.
         *
         * @param v
         * New coefficient.
         **************************************************************************/
        void SetDragZ(float v);

        /**************************************************************************
         * @brief
         * Computes angular drag torque based on current angular velocity.
         *
         * @param currentAngularVelocity
         * Current angular velocity.
         *
         * @return
         * Drag torque vector.
         **************************************************************************/
        Vector3D CalculateTorque(const Vector3D &currentAngularVelocity) const override;

        /**************************************************************************
         * @brief
         * Polymorphic clone.
         *
         * @return
         * Unique pointer to a copy.
         **************************************************************************/
        std::unique_ptr<Torque> Clone() const override;
    };

    /**************************************************************************
     * @brief
     * Aggregates and manages torques: add/clone, masking, lifetime, summation.
     **************************************************************************/
    class TorqueManager {
    private:
        std::vector<std::unique_ptr<Torque>> torques;
        Vector3D currentAngularVelocity;

        std::unordered_map<unsigned, std::string> maskToName;
        std::unordered_map<std::string, unsigned> nameToMask;

    public:
        /**************************************************************************
         * @brief
         * Default constructor.
         **************************************************************************/
        TorqueManager();
        ~TorqueManager() = default;

        /**************************************************************************
         * @brief
         * Copy constructor (deep-copies owned torques).
         *
         * @param other
         * Source manager.
         **************************************************************************/
        TorqueManager(const TorqueManager &other);

        /**************************************************************************
         * @brief
         * Move constructor.
         *
         * @param other
         * Source manager.
         **************************************************************************/
        TorqueManager(TorqueManager &&other) noexcept;

        /**************************************************************************
         * @brief
         * Copy assignment (deep-copies torques and state).
         *
         * @param other
         * Source manager.
         *
         * @return
         * Reference to this.
         **************************************************************************/
        TorqueManager &operator=(const TorqueManager &other);

        /**************************************************************************
         * @brief
         * Move assignment (transfers ownership).
         *
         * @param other
         * Source manager.
         *
         * @return
         * Reference to this.
         **************************************************************************/
        TorqueManager &operator=(TorqueManager &&other) noexcept;

        /**************************************************************************
         * @brief
         * Emplaces a new torque of type T with forwarded arguments.
         *
         * @tparam T
         * Torque type (derives from Torque).
         *
         * @tparam Args
         * Constructor parameter pack.
         **************************************************************************/
        template<typename T, typename... Args>
        void AddTorque(Args&&... args) {
            torques.push_back(std::make_unique<T>(std::forward<Args>(args)...));
        }

        /**************************************************************************
         * @brief
         * Clones and adds a torque instance.
         *
         * @param t
         * Source torque to clone.
         **************************************************************************/
        void AddTorque(const Torque &t);

        /**************************************************************************
         * @brief
         * Returns number of active torques.
         *
         * @return
         * Count of active torques.
         **************************************************************************/
        size_t GetActiveTorqueCount() const;

        /**************************************************************************
         * @brief
         * Returns total number of torques.
         *
         * @return
         * Count of all torques.
         **************************************************************************/
        size_t GetTotalTorqueCount() const;

        /**************************************************************************
         * @brief
         * Removes inactive Temp torques; preserves Perm torques.
         **************************************************************************/
        void CleanupTorques();

        /**************************************************************************
         * @brief
         * Removes torques whose masks intersect mask.
         *
         * @param mask
         * Filter mask.
         **************************************************************************/
        void RemoveTorquesByMask(unsigned mask);

        /**************************************************************************
         * @brief
         * Sets activation for torques matching mask.
         *
         * @param mask
         * Filter mask.
         *
         * @param active
         * New activation state.
         **************************************************************************/
        void ActivateByMask(unsigned mask, bool active);

        /**************************************************************************
         * @brief
         * Sets lifetime for torques matching mask (Temp only).
         *
         * @param mask
         * Filter mask.
         *
         * @param lifetime
         * Lifetime in seconds.
         **************************************************************************/
        void SetLifeTimeByMask(unsigned mask, float lifetime);

        /**************************************************************************
         * @brief
         * Accumulates active torques matching mask.
         *
         * @param mask
         * Filter mask.
         *
         * @return
         * Total torque vector.
         **************************************************************************/
        Vector3D CalculateTorqueByMask(unsigned mask) const;

        /**************************************************************************
         * @brief
         * Accumulates all active torques.
         *
         * @return
         * Total torque vector.
         **************************************************************************/
        Vector3D GetTotalTorque() const;

        /**************************************************************************
         * @brief
         * Sets current angular velocity sample for velocity-dependent torques.
         *
         * @param w
         * Current angular velocity.
         **************************************************************************/
        void SetCurrentAngularVelocity(const Vector3D &w);

        /**************************************************************************
         * @brief
         * Returns current angular velocity sample.
         *
         * @return
         * Const reference to angular velocity.
         **************************************************************************/
        const Vector3D &GetCurrentAngularVelocity() const;

        /**************************************************************************
         * @brief
         * Registers/overwrites a friendly name for a mask.
         *
         * @param mask
         * Mask value.
         *
         * @param name
         * Display name.
         **************************************************************************/
        void RegisterMaskName(unsigned mask, const std::string &name);

        /**************************************************************************
         * @brief
         * Removes mask<->name mapping.
         *
         * @param mask
         * Mask to unregister.
         *
         * @return
         * True if removed; false if not found.
         **************************************************************************/
        bool UnregisterMaskName(unsigned mask);

        /**************************************************************************
         * @brief
         * Returns name for a mask (empty if none).
         *
         * @param mask
         * Mask value.
         *
         * @return
         * Registered name or empty string.
         **************************************************************************/
        std::string GetMaskName(unsigned mask) const;

        /**************************************************************************
         * @brief
         * Finds a mask by its registered name.
         *
         * @param name
         * Mask name.
         *
         * @return
         * Mask value, or 0 if not found.
         **************************************************************************/
        unsigned FindMaskByName(const std::string &name) const;

        /**************************************************************************
         * @brief
         * Lists all registered (mask, name) pairs.
         *
         * @return
         * Vector of pairs.
         **************************************************************************/
        std::vector<std::pair<unsigned, std::string>> ListMaskNames() const;

        /**************************************************************************
         * @brief
         * Accumulates torques for a registered mask name.
         *
         * @param name
         * Registered mask name.
         *
         * @return
         * Total torque vector (zero if name not found).
         **************************************************************************/
        Vector3D CalculateTorqueByMaskName(const std::string &name) const;

        /**************************************************************************
         * @brief
         * Returns owned torques (const).
         *
         * @return
         * Const reference to torque container.
         **************************************************************************/
        const std::vector<std::unique_ptr<Torque>> &GetTorques() const;
    };

} // namespace gam300

#endif // __TORQUE_MANAGER_H__
