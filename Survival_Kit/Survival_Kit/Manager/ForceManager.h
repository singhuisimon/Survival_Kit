/*****************************************************************************/
/*!
\file       ForceManager.h
\author     Low Yue Jun (yuejun.low)
\par        email: yuejun.low@digipen.edu
\date       2025/10/03
\brief      Force framework:
            - Base Force with lifetime / activation / mask gating
            - LinearDirectionalForce and DragForce implementations
            - ForceManager for accumulation, masking, and lifecycle control

            (C) 2025 DigiPen Institute of Technology.
            Reproduction or disclosure of this file or its contents without the
            prior written consent of DigiPen Institute of Technology is prohibited.
*/
/*****************************************************************************/
#pragma once
#ifndef __FORCE_MANAGER_H__
#define __FORCE_MANAGER_H__

#include <algorithm>
#include <vector>
#include <utility>
#include <memory>
#include <unordered_map>
#include <string>
#include <cmath>
#include "../Utility/Vector3D.h"

namespace gam300 {
    /**************************************************************************
     * @brief
     * Base interface for linear forces with lifetime, activation, and masks.
     **************************************************************************/
    class Force {
    public:
        enum DURATION {
            Temp, Perm, Impulse
        };

    protected:
        DURATION durationType{};
        float    lifetime{};
        bool     isActive{};
        float    age{};
        unsigned forceMask{};

    public:
        /**************************************************************************
         * @brief
         * Constructs a force with duration type and category mask.
         *
         * @param type
         * Duration category (Temp, Perm, Impulse).
         * @param mask
         * Bitmask for selection/grouping.
         **************************************************************************/
        Force(DURATION type, unsigned mask);
        virtual ~Force() = default;

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
         * Sets lifetime for Temp forces (ignored for Perm).
         *
         * @param value
         * Lifetime in seconds.
         **************************************************************************/
        void SetLifetime(float value);

        /**************************************************************************
         * @brief
         * Returns whether this force is active.
         *
         * @return
         * Activation flag.
         **************************************************************************/
        bool GetIsActive() const;

        /**************************************************************************
         * @brief
         * Enables or disables this force.
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
        unsigned GetForceMask() const;

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
         * Deactivates Temp forces if age exceeds lifetime.
         *
         * @return
         * False if expired and deactivated; true otherwise.
         **************************************************************************/
        bool ValidateAge();

        /**************************************************************************
         * @brief
         * Advances internal age and validates lifetime for Temp forces.
         *
         * @param deltaTime
         * Time step in seconds.
         **************************************************************************/
        void Update(double deltaTime);

        /**************************************************************************
         * @brief
         * Computes force contribution using current velocity sample.
         *
         * @param currentVelocity
         * Velocity of the body (for velocity-dependent forces).
         *
         * @return
         * Force vector to accumulate.
         **************************************************************************/
        virtual Vector3D CalculateForce(const Vector3D &currentVelocity) const = 0;

        /**************************************************************************
         * @brief
         * Polymorphic clone.
         *
         * @return
         * Unique pointer to a copy of this force.
         **************************************************************************/
        virtual std::unique_ptr<Force> Clone() const = 0;
    };

    /**************************************************************************
     * @brief
     * Constant-direction linear force: F = dir * magnitude.
     **************************************************************************/
    class LinearDirectionalForce : public Force {
    private:
        Vector3D unitDirection{};
        float    magnitude{};

    public:
        /**************************************************************************
         * @brief
         * Constructs a constant-direction force.
         *
         * @param direction
         * Direction vector (normalized internally).
         * @param magnitude
         * Scalar magnitude.
         * @param mask
         * Category mask.
         * @param type
         * Duration category.
         **************************************************************************/
        LinearDirectionalForce(const Vector3D &direction,
                               float           magnitude,
                               unsigned        mask,
                               DURATION        type);

        /**************************************************************************
         * @brief
         * Returns the unit direction of the force.
         *
         * @return
         * Unit direction vector.
         **************************************************************************/
        Vector3D GetUnitDirection() const;

        /**************************************************************************
         * @brief
         * Sets the force direction (normalized internally).
         *
         * @param direction
         * New direction vector.
         **************************************************************************/
        void SetUnitDirection(const Vector3D &direction);

        /**************************************************************************
         * @brief
         * Returns magnitude.
         *
         * @return
         * Scalar magnitude.
         **************************************************************************/
        float GetMagnitude() const;

        /**************************************************************************
         * @brief
         * Sets magnitude.
         *
         * @param value
         * New magnitude.
         **************************************************************************/
        void SetMagnitude(float value);

        /**************************************************************************
         * @brief
         * Computes the force contribution.
         *
         * @param currentVelocity
         * Current velocity (unused by this force type).
         *
         * @return
         * Force vector.
         **************************************************************************/
        Vector3D CalculateForce(const Vector3D &currentVelocity) const override;

        /**************************************************************************
         * @brief
         * Polymorphic clone.
         *
         * @return
         * Unique pointer to a copy.
         **************************************************************************/
        std::unique_ptr<Force> Clone() const override;
    };

    /**************************************************************************
     * @brief
     * Anisotropic drag force: F = -c * v (X,Z use horizontal; Y uses vertical).
     **************************************************************************/
    class DragForce : public Force {
    private:
        float horizontalDragCoeff{};
        float verticalDragCoeff{};

    public:
        /**************************************************************************
         * @brief
         * Constructs anisotropic drag.
         *
         * @param hDrag
         * Horizontal (X,Z) coefficient.
         * @param vDrag
         * Vertical (Y) coefficient.
         * @param mask
         * Category mask.
         * @param type
         * Duration category (default Perm).
         **************************************************************************/
        explicit DragForce(float hDrag, float vDrag, unsigned mask,
                           DURATION type = DURATION::Perm);

        /**************************************************************************
         * @brief
         * Returns horizontal drag coefficient.
         *
         * @return
         * Coefficient value.
         **************************************************************************/
        float GetHorizontalDragCoefficient() const;

        /**************************************************************************
         * @brief
         * Sets horizontal drag coefficient.
         *
         * @param value
         * New coefficient.
         **************************************************************************/
        void SetHorizontalDragCoefficient(float value);

        /**************************************************************************
         * @brief
         * Returns vertical drag coefficient.
         *
         * @return
         * Coefficient value.
         **************************************************************************/
        float GetVerticalDragCoefficient() const;

        /**************************************************************************
         * @brief
         * Sets vertical drag coefficient.
         *
         * @param value
         * New coefficient.
         **************************************************************************/
        void SetVerticalDragCoefficient(float value);

        /**************************************************************************
         * @brief
         * Computes drag force.
         *
         * @param currentVelocity
         * Current linear velocity.
         *
         * @return
         * Drag force vector.
         **************************************************************************/
        Vector3D CalculateForce(const Vector3D &currentVelocity) const override;

        /**************************************************************************
         * @brief
         * Polymorphic clone.
         *
         * @return
         * Unique pointer to a copy.
         **************************************************************************/
        std::unique_ptr<Force> Clone() const override;
    };

    /**************************************************************************
     * @brief
     * Aggregates and manages forces: add/clone, masking, lifetime, summation.
     **************************************************************************/
    class ForceManager {
    private:
        std::vector<std::unique_ptr<Force>> forces{};
        Vector3D currentVelocity{ 0.0f, 0.0f, 0.0f };

        std::unordered_map<unsigned, std::string> maskToName{};
        std::unordered_map<std::string, unsigned> nameToMask{};

    public:
        /**************************************************************************
         * @brief
         * Default constructor.
         **************************************************************************/
        ForceManager();
        ~ForceManager() = default;

        /**************************************************************************
         * @brief
         * Copy constructor (deep-copies owned forces).
         *
         * @param other
         * Source manager.
         **************************************************************************/
        ForceManager(const ForceManager &other);

        /**************************************************************************
         * @brief
         * Move constructor.
         *
         * @param other
         * Source manager.
         **************************************************************************/
        ForceManager(ForceManager &&other) noexcept;

        /**************************************************************************
         * @brief
         * Copy assignment (deep-copies forces and state).
         *
         * @param other
         * Source manager.
         *
         * @return
         * Reference to this.
         **************************************************************************/
        ForceManager &operator=(const ForceManager &other);

        /**************************************************************************
         * @brief
         * Move assignment.
         *
         * @param other
         * Source manager.
         *
         * @return
         * Reference to this.
         **************************************************************************/
        ForceManager &operator=(ForceManager &&other) noexcept;

        /**************************************************************************
         * @brief
         * Emplaces a new force of type T with forwarded arguments.
         *
         * @tparam T
         * Force type (derives from Force).
         * @tparam Args
         * Constructor parameter pack.
         **************************************************************************/
        template<typename T, typename... Args>
        void AddForce(Args&&... args) {
            forces.push_back(std::make_unique<T>(std::forward<Args>(args)...));
        }

        /**************************************************************************
         * @brief
         * Clones and adds a force instance.
         *
         * @param force
         * Source force to clone.
         **************************************************************************/
        void AddForce(const Force &force);

        /**************************************************************************
         * @brief
         * Returns number of active forces.
         *
         * @return
         * Count of active forces.
         **************************************************************************/
        size_t GetActiveForceCount() const;

        /**************************************************************************
         * @brief
         * Returns total number of forces.
         *
         * @return
         * Count of all forces.
         **************************************************************************/
        size_t GetTotalForceCount() const;

        /**************************************************************************
         * @brief
         * Removes inactive Temp forces; preserves Perm forces.
         **************************************************************************/
        void CleanupForces();

        /**************************************************************************
         * @brief
         * Removes forces whose masks intersect the given mask.
         *
         * @param mask
         * Filter mask.
         **************************************************************************/
        void RemoveForcesByMask(unsigned mask);

        /**************************************************************************
         * @brief
         * Sets activation for forces matching a mask.
         *
         * @param mask
         * Filter mask.
         * @param active
         * New activation state.
         **************************************************************************/
        void ActivateByMask(unsigned mask, bool active);

        /**************************************************************************
         * @brief
         * Sets lifetime for forces matching a mask (Temp only).
         *
         * @param mask
         * Filter mask.
         * @param lifetime
         * New lifetime in seconds.
         **************************************************************************/
        void SetLifeTimeByMask(unsigned mask, float lifetime);

        /**************************************************************************
         * @brief
         * Accumulates forces that are active and match the given mask.
         *
         * @param mask
         * Filter mask.
         *
         * @return
         * Total accumulated force.
         **************************************************************************/
        Vector3D CalculateForceByMask(unsigned mask) const;

        /**************************************************************************
         * @brief
         * Accumulates all active forces.
         *
         * @return
         * Total accumulated force.
         **************************************************************************/
        Vector3D GetTotalForce() const;

        /**************************************************************************
         * @brief
         * Sets the current velocity sample for velocity-dependent forces.
         *
         * @param velocity
         * Current linear velocity.
         **************************************************************************/
        void SetCurrentVelocity(const Vector3D &velocity);

        /**************************************************************************
         * @brief
         * Returns the current velocity sample.
         *
         * @return
         * Const reference to velocity.
         **************************************************************************/
        const Vector3D &GetCurrentVelocity() const;

        /**************************************************************************
         * @brief
         * Registers (or overwrites) a friendly name for a mask.
         *
         * @param mask
         * Mask value.
         * @param name
         * Display name to associate.
         **************************************************************************/
        void RegisterMaskName(unsigned mask, const std::string &name);

        /**************************************************************************
         * @brief
         * Removes mask<->name mapping for a mask (both directions).
         *
         * @param mask
         * Mask value to unregister.
         *
         * @return
         * True if removed; false if not found.
         **************************************************************************/
        bool UnregisterMaskName(unsigned mask);

        /**************************************************************************
         * @brief
         * Fetches the display name for a mask (empty if none).
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
         * Finds a mask by its registered name (0 if not found).
         *
         * @param name
         * Registered mask name.
         *
         * @return
         * Mask value.
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
         * Accumulates forces for a registered mask name.
         *
         * @param name
         * Registered mask name.
         *
         * @return
         * Total accumulated force (zero if name not found).
         **************************************************************************/
        Vector3D CalculateForceByMaskName(const std::string &name) const;

        /**************************************************************************
         * @brief
         * Returns owned forces (const).
         *
         * @return
         * Const reference to force container.
         **************************************************************************/
        std::vector<std::unique_ptr<Force>> const &GetForces() const;
    };

} // namespace gam300

#endif // __FORCE_MANAGER_H__
