/*****************************************************************************/
/*!
\file       ForceManager.cpp
\author     Low Yue Jun (yuejun.low)
\par        email: yuejun.low@digipen.edu
\date       2025/10/03
\brief      Force system implementation:
            - Base Force with lifetime, activation, and mask gating
            - LinearDirectionalForce and DragForce implementations
            - ForceManager for accumulation, masking, and lifecycle control

            (C) 2025 DigiPen Institute of Technology.
            Reproduction or disclosure of this file or its contents without the
            prior written consent of DigiPen Institute of Technology is prohibited.
*/
/*****************************************************************************/

#include "ForceManager.h"

#include <algorithm>
#include <cmath>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace gam300 {
    /**************************************************************************
     * @brief
     * Safely normalizes a vector; returns zero if magnitude is near zero.
     *
     * @param v
     * Input vector.
     *
     * @return
     * Unit vector in direction of v, or zero vector if too small.
     **************************************************************************/
    static inline Vector3D NormalizeSafe(Vector3D const &v) {
        float const len2{ v.x * v.x + v.y * v.y + v.z * v.z };
        if(len2 <= 1e-12f) {
            return Vector3D{ 0.0f, 0.0f, 0.0f };
        }
        float const invLen{ 1.0f / std::sqrt(len2) };
        return Vector3D{ v.x * invLen, v.y * invLen, v.z * invLen };
    }

    /**************************************************************************
     * @brief
     * Constructs a force with duration type and mask.
     *
     * @param type
     * Duration category (Temp or Perm).
     * @param mask
     * Mask bits indicating which groups this force belongs to.
     **************************************************************************/
    Force::Force(DURATION type, unsigned mask) :
        durationType{ type },
        lifetime{ 0.0f },
        isActive{ true },
        age{ 0.0f },
        forceMask{ mask } {
        if(durationType == Perm) {
            lifetime = -1.0f;
        }
    }

    /**************************************************************************
     * @brief
     * Returns configured lifetime in seconds (-1 for Perm).
     *
     * @return
     * Lifetime value.
     **************************************************************************/
    float Force::GetLifetime() const {
        return lifetime;
    }

    /**************************************************************************
     * @brief
     * Sets lifetime if duration is Temp; ignored for Perm.
     *
     * @param value
     * Lifetime in seconds.
     **************************************************************************/
    void Force::SetLifetime(float value) {
        if(durationType != Perm) lifetime = value;
    }

    /**************************************************************************
     * @brief
     * Returns whether the force is currently active.
     *
     * @return
     * Activation flag.
     **************************************************************************/
    bool Force::GetIsActive() const {
        return isActive;
    }

    /**************************************************************************
     * @brief
     * Enables or disables this force.
     *
     * @param value
     * New activation state.
     **************************************************************************/
    void Force::SetIsActive(bool value) {
        isActive = value;
    }

    /**************************************************************************
     * @brief
     * Returns current age in seconds.
     *
     * @return
     * Accumulated age.
     **************************************************************************/
    float Force::GetAge() const {
        return age;
    }

    /**************************************************************************
     * @brief
     * Sets the current age in seconds.
     *
     * @param value
     * New age.
     **************************************************************************/
    void Force::SetAge(float value) {
        age = value;
    }

    /**************************************************************************
     * @brief
     * Returns the mask bits for this force.
     *
     * @return
     * Mask value.
     **************************************************************************/
    unsigned Force::GetForceMask() const {
        return forceMask;
    }

    /**************************************************************************
     * @brief
     * Returns the duration type (Temp/Perm).
     *
     * @return
     * Duration type.
     **************************************************************************/
    Force::DURATION Force::GetDurationType() const {
        return durationType;
    }

    /**************************************************************************
     * @brief
     * Validates expiration for Temp forces and deactivates if expired.
     *
     * @return
     * False if expired and deactivated; true otherwise.
     **************************************************************************/
    bool Force::ValidateAge() {
        if(durationType != Perm && age >= lifetime) {
            isActive = false;
            return false;
        }
        return true;
    }

    /**************************************************************************
     * @brief
     * Updates the internal age and validates lifetime for Temp forces.
     *
     * @param deltaTime
     * Time step (seconds).
     **************************************************************************/
    void Force::Update(double deltaTime) {
        if(durationType != Perm && isActive) {
            age += static_cast<float>(deltaTime);
            if(!ValidateAge()) SetAge(0.0f);
        }
    }

    /**************************************************************************
     * @brief
     * Constructs a constant-direction linear force.
     *
     * @param direction
     * Direction to apply (will be normalized safely).
     * @param mag
     * Magnitude of the force.
     * @param mask
     * Mask bits.
     * @param type
     * Duration type (Temp/Perm).
     **************************************************************************/
    LinearDirectionalForce::LinearDirectionalForce(Vector3D const &direction,
                                                   float           mag,
                                                   unsigned        mask,
                                                   DURATION        type) :
        Force{ type, mask },
        unitDirection{ NormalizeSafe(direction) },
        magnitude{ mag } {}

    /**************************************************************************
     * @brief
     * Returns the unit direction of the force.
     *
     * @return
     * Unit vector.
     **************************************************************************/
    Vector3D LinearDirectionalForce::GetUnitDirection() const {
        return unitDirection;
    }

    /**************************************************************************
     * @brief
     * Sets the direction (normalized safely).
     *
     * @param direction
     * New direction vector.
     **************************************************************************/
    void LinearDirectionalForce::SetUnitDirection(Vector3D const &direction) {
        unitDirection = NormalizeSafe(direction);
    }

    /**************************************************************************
     * @brief
     * Returns the magnitude of the force.
     *
     * @return
     * Scalar magnitude.
     **************************************************************************/
    float LinearDirectionalForce::GetMagnitude() const {
        return magnitude;
    }

    /**************************************************************************
     * @brief
     * Sets the magnitude of the force.
     *
     * @param value
     * New magnitude.
     **************************************************************************/
    void LinearDirectionalForce::SetMagnitude(float value) {
        magnitude = value;
    }

    /**************************************************************************
     * @brief
     * Computes the linear force contribution.
     *
     * @param currentVelocity
     * Current velocity (unused by this force type).
     *
     * @return
     * Force vector.
     **************************************************************************/
    Vector3D LinearDirectionalForce::CalculateForce(Vector3D const &/*currentVelocity*/) const {
        return Vector3D{ unitDirection.x * magnitude,
                         unitDirection.y * magnitude,
                         unitDirection.z * magnitude };
    }

    /**************************************************************************
     * @brief
     * Polymorphic clone.
     *
     * @return
     * Unique pointer to a copy.
     **************************************************************************/
    std::unique_ptr<Force> LinearDirectionalForce::Clone() const {
        return std::make_unique<LinearDirectionalForce>(*this);
    }

    /**************************************************************************
     * @brief
     * Constructs an anisotropic linear drag force.
     *
     * @param hDrag
     * Horizontal (X,Z) drag coefficient.
     * @param vDrag
     * Vertical (Y) drag coefficient.
     * @param mask
     * Mask bits.
     * @param type
     * Duration type (Temp/Perm).
     **************************************************************************/
    DragForce::DragForce(float hDrag, float vDrag, unsigned mask, DURATION type) :
        Force{ type, mask },
        horizontalDragCoeff{ hDrag },
        verticalDragCoeff{ vDrag } {
        if(type == Perm) {
            lifetime = -1.0f;
        }
    }

    /**************************************************************************
     * @brief
     * Returns horizontal drag coefficient (X,Z).
     *
     * @return
     * Coefficient value.
     **************************************************************************/
    float DragForce::GetHorizontalDragCoefficient() const {
        return horizontalDragCoeff;
    }

    /**************************************************************************
     * @brief
     * Sets horizontal drag coefficient (X,Z).
     *
     * @param value
     * New coefficient.
     **************************************************************************/
    void DragForce::SetHorizontalDragCoefficient(float value) {
        horizontalDragCoeff = value;
    }

    /**************************************************************************
     * @brief
     * Returns vertical drag coefficient (Y).
     *
     * @return
     * Coefficient value.
     **************************************************************************/
    float DragForce::GetVerticalDragCoefficient() const {
        return verticalDragCoeff;
    }

    /**************************************************************************
     * @brief
     * Sets vertical drag coefficient (Y).
     *
     * @param value
     * New coefficient.
     **************************************************************************/
    void DragForce::SetVerticalDragCoefficient(float value) {
        verticalDragCoeff = value;
    }

    /**************************************************************************
     * @brief
     * Computes anisotropic drag based on current velocity.
     *
     * @param currentVelocity
     * Current linear velocity.
     *
     * @return
     * Drag force vector.
     **************************************************************************/
    Vector3D DragForce::CalculateForce(Vector3D const &currentVelocity) const {
        return Vector3D{
            -currentVelocity.x * horizontalDragCoeff,
            -currentVelocity.y * verticalDragCoeff,
            -currentVelocity.z * horizontalDragCoeff
        };
    }

    /**************************************************************************
     * @brief
     * Polymorphic clone.
     *
     * @return
     * Unique pointer to a copy.
     **************************************************************************/
    std::unique_ptr<Force> DragForce::Clone() const {
        return std::make_unique<DragForce>(*this);
    }

    /**************************************************************************
     * @brief
     * Default-constructs a manager with zero current velocity.
     **************************************************************************/
    ForceManager::ForceManager() :
        currentVelocity{ 0.0f, 0.0f, 0.0f } {}

    /**************************************************************************
     * @brief
     * Copy-constructs, deep-copying owned forces.
     *
     * @param other
     * Source manager.
     **************************************************************************/
    ForceManager::ForceManager(ForceManager const &other) :
        currentVelocity{ other.currentVelocity } {
        for(auto const &f : other.forces) {
            forces.push_back(f->Clone());
        }
    }

    /**************************************************************************
     * @brief
     * Move-constructs, transferring ownership of forces.
     *
     * @param other
     * Source manager.
     **************************************************************************/
    ForceManager::ForceManager(ForceManager &&other) noexcept :
        forces{ std::move(other.forces) },
        currentVelocity{ other.currentVelocity } {}

    /**************************************************************************
     * @brief
     * Copy assignment; deep-copies forces and state.
     *
     * @param other
     * Source manager.
     *
     * @return
     * Reference to this.
     **************************************************************************/
    ForceManager &ForceManager::operator=(ForceManager const &other) {
        if(this != &other) {
            forces.clear();
            currentVelocity = other.currentVelocity;
            for(auto const &f : other.forces) {
                forces.push_back(f->Clone());
            }
        }
        return *this;
    }

    /**************************************************************************
     * @brief
     * Move assignment; transfers ownership of forces and state.
     *
     * @param other
     * Source manager.
     *
     * @return
     * Reference to this.
     **************************************************************************/
    ForceManager &ForceManager::operator=(ForceManager &&other) noexcept {
        if(this != &other) {
            forces = std::move(other.forces);
            currentVelocity = other.currentVelocity;
        }
        return *this;
    }

    /**************************************************************************
     * @brief
     * Adds a force by cloning the given instance.
     *
     * @param force
     * Source force to clone and store.
     **************************************************************************/
    void ForceManager::AddForce(Force const &force) {
        forces.push_back(force.Clone());
    }

    /**************************************************************************
     * @brief
     * Returns the number of active forces.
     *
     * @return
     * Count of active forces.
     **************************************************************************/
    size_t ForceManager::GetActiveForceCount() const {
        return static_cast<size_t>(std::count_if(
            forces.begin(), forces.end(),
            [](std::unique_ptr<Force> const &f) { return f->GetIsActive(); }));
    }

    /**************************************************************************
     * @brief
     * Returns the total number of forces.
     *
     * @return
     * Count of all forces.
     **************************************************************************/
    size_t ForceManager::GetTotalForceCount() const {
        return forces.size();
    }

    /**************************************************************************
     * @brief
     * Removes inactive Temp forces; preserves Perm forces.
     **************************************************************************/
    void ForceManager::CleanupForces() {
        forces.erase(
            std::remove_if(
            forces.begin(), forces.end(),
            [](std::unique_ptr<Force> const &f) {
                return (!f->GetIsActive() && f->GetDurationType() == Force::Temp);
            }),
            forces.end());
    }

    /**************************************************************************
     * @brief
     * Removes forces whose masks intersect the given mask.
     *
     * @param mask
     * Mask for removal filter.
     **************************************************************************/
    void ForceManager::RemoveForcesByMask(unsigned mask) {
        forces.erase(
            std::remove_if(
            forces.begin(), forces.end(),
            [mask](std::unique_ptr<Force> const &f) {
                return (f->GetForceMask() & mask) != 0u;
            }),
            forces.end());
    }

    /**************************************************************************
     * @brief
     * Sets activation for forces matching a mask.
     *
     * @param mask
     * Filter mask.
     * @param active
     * New activation state.
     **************************************************************************/
    void ForceManager::ActivateByMask(unsigned mask, bool active) {
        for(auto &f : forces) {
            if((f->GetForceMask() & mask) != 0u) {
                f->SetIsActive(active);
            }
        }
    }

    /**************************************************************************
     * @brief
     * Sets lifetime for forces matching a mask (Temp only).
     *
     * @param mask
     * Filter mask.
     * @param lifetimeInSeconds
     * New lifetime.
     **************************************************************************/
    void ForceManager::SetLifeTimeByMask(unsigned mask, float lifetimeInSeconds) {
        for(auto &f : forces) {
            if((f->GetForceMask() & mask) != 0u) {
                f->SetLifetime(lifetimeInSeconds);
            }
        }
    }

    /**************************************************************************
     * @brief
     * Accumulates forces that are active and match the given mask.
     *
     * @param mask
     * Filter mask.
     *
     * @return
     * Total force vector.
     **************************************************************************/
    Vector3D ForceManager::CalculateForceByMask(unsigned mask) const {
        Vector3D total{ 0.0f, 0.0f, 0.0f };
        for(auto const &f : forces) {
            if(f->GetIsActive() && (f->GetForceMask() & mask) != 0u) {
                Vector3D const c{ f->CalculateForce(currentVelocity) };
                total.x += c.x; total.y += c.y; total.z += c.z;
            }
        }
        return total;
    }

    /**************************************************************************
     * @brief
     * Accumulates all active forces regardless of mask.
     *
     * @return
     * Total force vector.
     **************************************************************************/
    Vector3D ForceManager::GetTotalForce() const {
        Vector3D total{ 0.0f, 0.0f, 0.0f };
        for(auto const &f : forces) {
            if(f->GetIsActive()) {
                Vector3D const c{ f->CalculateForce(currentVelocity) };
                total.x += c.x; total.y += c.y; total.z += c.z;
            }
        }
        return total;
    }

    /**************************************************************************
     * @brief
     * Sets the current velocity used by velocity-dependent forces.
     *
     * @param velocity
     * Current linear velocity.
     **************************************************************************/
    void ForceManager::SetCurrentVelocity(Vector3D const &velocity) {
        currentVelocity = velocity;
    }

    /**************************************************************************
     * @brief
     * Returns the current velocity sample.
     *
     * @return
     * Reference to current velocity.
     **************************************************************************/
    Vector3D const &ForceManager::GetCurrentVelocity() const {
        return currentVelocity;
    }

    /**************************************************************************
     * @brief
     * Registers a friendly name for a mask value.
     *
     * @param mask
     * Mask value.
     * @param name
     * Name to associate.
     **************************************************************************/
    void ForceManager::RegisterMaskName(unsigned mask, std::string const &name) {
        maskToName[mask] = name;
        nameToMask[name] = mask;
    }

    /**************************************************************************
     * @brief
     * Unregisters a mask name mapping by mask.
     *
     * @param mask
     * Mask value to remove.
     *
     * @return
     * True if removed; false if not found.
     **************************************************************************/
    bool ForceManager::UnregisterMaskName(unsigned mask) {
        auto it{ maskToName.find(mask) };
        if(it == maskToName.end()) {
            return false;
        }
        std::string const old{ it->second };
        maskToName.erase(it);
        auto it2{ nameToMask.find(old) };
        if(it2 != nameToMask.end()) {
            nameToMask.erase(it2);
        }
        return true;
    }

    /**************************************************************************
     * @brief
     * Looks up the friendly name for a mask.
     *
     * @param mask
     * Mask value.
     *
     * @return
     * Name if found; empty string otherwise.
     **************************************************************************/
    std::string ForceManager::GetMaskName(unsigned mask) const {
        auto it{ maskToName.find(mask) };
        return (it == maskToName.end()) ? std::string{} : it->second;
    }

    /**************************************************************************
     * @brief
     * Finds a mask value by its registered name.
     *
     * @param name
     * Mask name.
     *
     * @return
     * Mask value if found; 0 otherwise.
     **************************************************************************/
    unsigned ForceManager::FindMaskByName(std::string const &name) const {
        auto it{ nameToMask.find(name) };
        return (it == nameToMask.end()) ? 0u : it->second;
    }

    /**************************************************************************
     * @brief
     * Returns a list of all registered mask-name pairs.
     *
     * @return
     * Vector of (mask, name) pairs.
     **************************************************************************/
    std::vector<std::pair<unsigned, std::string>> ForceManager::ListMaskNames() const {
        std::vector<std::pair<unsigned, std::string>> out{};
        out.reserve(maskToName.size());
        for(auto const &kv : maskToName) {
            out.emplace_back(kv.first, kv.second);
        }
        return out;
    }

    /**************************************************************************
     * @brief
     * Accumulates forces matching the given mask name.
     *
     * @param name
     * Registered mask name.
     *
     * @return
     * Total force vector, or zero if name not found.
     **************************************************************************/
    Vector3D ForceManager::CalculateForceByMaskName(std::string const &name) const {
        unsigned const mask{ FindMaskByName(name) };
        if(mask == 0u) {
            return Vector3D{ 0.0f, 0.0f, 0.0f };
        }
        return CalculateForceByMask(mask);
    }

    /**************************************************************************
     * @brief
     * Returns the internal force container (const).
     *
     * @return
     * Const reference to owned forces.
     **************************************************************************/
    std::vector<std::unique_ptr<Force>> const &ForceManager::GetForces() const {
        return forces;
    }

} // namespace gam300
