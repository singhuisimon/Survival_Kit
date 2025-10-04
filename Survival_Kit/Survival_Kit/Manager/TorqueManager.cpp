/*****************************************************************************/
/*!
\file       TorqueManager.cpp
\author     Low Yue Jun (yuejun.low)
\par        email: yuejun.low@digipen.edu
\date       2025/10/03
\brief      Implementation for torque framework:
            - Base Torque with lifetime/activation/mask
            - AngularDirectionalTorque and AngularDrag
            - TorqueManager aggregation, masking, and lifecycle control

            (C) 2025 DigiPen Institute of Technology.
            Reproduction or disclosure of this file or its contents without the
            prior written consent of DigiPen Institute of Technology is prohibited.
*/
/*****************************************************************************/

#include "TorqueManager.h"
#include <cmath>

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
     * Constructs a torque with duration type and mask.
     *
     * @param type
     * Duration category (Temp or Perm).
     * @param mask
     * Mask bits indicating which groups this torque belongs to.
     **************************************************************************/
    Torque::Torque(DURATION type, unsigned mask) :
        durationType{ type },
        lifetime{ 0.0f },
        isActive{ true },
        age{ 0.0f },
        torqueMask{ mask } {
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
    float Torque::GetLifetime() const {
        return lifetime;
    }

    /**************************************************************************
     * @brief
     * Sets lifetime if duration is Temp; ignored for Perm.
     *
     * @param value
     * Lifetime in seconds.
     **************************************************************************/
    void Torque::SetLifetime(float value) {
        if(durationType != Perm) {
            lifetime = value;
        }
    }

    /**************************************************************************
     * @brief
     * Returns whether the torque is currently active.
     *
     * @return
     * Activation flag.
     **************************************************************************/
    bool Torque::GetIsActive() const {
        return isActive;
    }

    /**************************************************************************
     * @brief
     * Enables or disables this torque.
     *
     * @param value
     * New activation state.
     **************************************************************************/
    void Torque::SetIsActive(bool value) {
        isActive = value;
    }

    /**************************************************************************
     * @brief
     * Returns current age in seconds.
     *
     * @return
     * Accumulated age.
     **************************************************************************/
    float Torque::GetAge() const {
        return age;
    }

    /**************************************************************************
     * @brief
     * Sets the current age in seconds.
     *
     * @param value
     * New age.
     **************************************************************************/
    void Torque::SetAge(float value) {
        age = value;
    }

    /**************************************************************************
     * @brief
     * Returns the mask bits for this torque.
     *
     * @return
     * Mask value.
     **************************************************************************/
    unsigned Torque::GetTorqueMask() const {
        return torqueMask;
    }

    /**************************************************************************
     * @brief
     * Returns the duration type (Temp/Perm).
     *
     * @return
     * Duration type.
     **************************************************************************/
    Torque::DURATION Torque::GetDurationType() const {
        return durationType;
    }

    /**************************************************************************
     * @brief
     * Validates expiration for Temp torques and deactivates if expired.
     *
     * @return
     * False if expired and deactivated; true otherwise.
     **************************************************************************/
    bool Torque::ValidateAge() {
        if(durationType != Perm && age >= lifetime) {
            isActive = false;
            return false;
        }
        return true;
    }

    /**************************************************************************
     * @brief
     * Updates the internal age and validates lifetime for Temp torques.
     *
     * @param deltaTime
     * Time step (seconds).
     **************************************************************************/
    void Torque::Update(double deltaTime) {
        if(durationType != Perm && isActive) {
            age += static_cast<float>(deltaTime);
            if(!ValidateAge()) {
                SetAge(0.0f);
            }
        }
    }

    /**************************************************************************
     * @brief
     * Constructs a constant-axis torque.
     *
     * @param axisUnit
     * Axis (normalized safely).
     * @param mag
     * Magnitude of the torque.
     * @param mask
     * Mask bits.
     * @param type
     * Duration type (Temp/Perm).
     **************************************************************************/
    AngularDirectionalTorque::AngularDirectionalTorque(Vector3D const &axisUnit,
                                                       float           mag,
                                                       unsigned        mask,
                                                       DURATION        type) :
        Torque{ type, mask },
        unitAxis{ NormalizeSafe(axisUnit) },
        magnitude{ mag } {}

    /**************************************************************************
     * @brief
     * Returns the unit axis of the torque.
     *
     * @return
     * Unit axis vector.
     **************************************************************************/
    Vector3D AngularDirectionalTorque::GetUnitAxis() const {
        return unitAxis;
    }

    /**************************************************************************
     * @brief
     * Sets the axis (normalized safely).
     *
     * @param axis
     * New axis vector.
     **************************************************************************/
    void AngularDirectionalTorque::SetUnitAxis(Vector3D const &axis) {
        unitAxis = NormalizeSafe(axis);
    }

    /**************************************************************************
     * @brief
     * Returns the magnitude of the torque.
     *
     * @return
     * Scalar magnitude.
     **************************************************************************/
    float AngularDirectionalTorque::GetMagnitude() const {
        return magnitude;
    }

    /**************************************************************************
     * @brief
     * Sets the magnitude of the torque.
     *
     * @param value
     * New magnitude.
     **************************************************************************/
    void AngularDirectionalTorque::SetMagnitude(float value) {
        magnitude = value;
    }

    /**************************************************************************
     * @brief
     * Computes the torque contribution.
     *
     * @param currentAngularVelocity
     * Current angular velocity (unused for this type).
     *
     * @return
     * Torque vector.
     **************************************************************************/
    Vector3D AngularDirectionalTorque::CalculateTorque(Vector3D const &/*currentAngularVelocity*/) const {
        return Vector3D{ unitAxis.x * magnitude,
                         unitAxis.y * magnitude,
                         unitAxis.z * magnitude };
    }

    /**************************************************************************
     * @brief
     * Polymorphic clone.
     *
     * @return
     * Unique pointer to a copy.
     **************************************************************************/
    std::unique_ptr<Torque> AngularDirectionalTorque::Clone() const {
        return std::make_unique<AngularDirectionalTorque>(*this);
    }

    /**************************************************************************
     * @brief
     * Constructs per-axis angular drag.
     *
     * @param cx
     * Drag coefficient for X.
     * @param cy
     * Drag coefficient for Y.
     * @param cz
     * Drag coefficient for Z.
     * @param mask
     * Mask bits.
     * @param type
     * Duration type (Temp/Perm).
     **************************************************************************/
    AngularDrag::AngularDrag(float cx, float cy, float cz, unsigned mask, DURATION type) :
        Torque{ type, mask },
        dragX{ cx },
        dragY{ cy },
        dragZ{ cz } {
        if(type == Perm) {
            lifetime = -1.0f;
        }
    }

    /**************************************************************************
     * @brief Returns X-axis drag coefficient.
     *
     * @return
     * Coefficient value.
     **************************************************************************/
    float AngularDrag::GetDragX() const {
        return dragX;
    }

    /**************************************************************************
     * @brief Returns Y-axis drag coefficient.
     *
     * @return
     * Coefficient value.
     **************************************************************************/
    float AngularDrag::GetDragY() const {
        return dragY;
    }

    /**************************************************************************
     * @brief Returns Z-axis drag coefficient.
     *
     * @return
     * Coefficient value.
     **************************************************************************/
    float AngularDrag::GetDragZ() const {
        return dragZ;
    }

    /**************************************************************************
     * @brief Sets X-axis drag coefficient.
     *
     * @param v
     * New coefficient.
     **************************************************************************/
    void AngularDrag::SetDragX(float v) {
        dragX = v;
    }

    /**************************************************************************
     * @brief Sets Y-axis drag coefficient.
     *
     * @param v
     * New coefficient.
     **************************************************************************/
    void AngularDrag::SetDragY(float v) {
        dragY = v;
    }

    /**************************************************************************
     * @brief Sets Z-axis drag coefficient.
     *
     * @param v
     * New coefficient.
     **************************************************************************/
    void AngularDrag::SetDragZ(float v) {
        dragZ = v;
    }

    /**************************************************************************
     * @brief
     * Computes angular drag torque based on current angular velocity.
     *
     * @param w
     * Current angular velocity.
     *
     * @return
     * Drag torque vector.
     **************************************************************************/
    Vector3D AngularDrag::CalculateTorque(Vector3D const &w) const {
        return Vector3D{ -dragX * w.x, -dragY * w.y, -dragZ * w.z };
    }

    /**************************************************************************
     * @brief
     * Polymorphic clone.
     *
     * @return
     * Unique pointer to a copy.
     **************************************************************************/
    std::unique_ptr<Torque> AngularDrag::Clone() const {
        return std::make_unique<AngularDrag>(*this);
    }

    /**************************************************************************
     * @brief
     * Default-constructs a manager with zero current angular velocity.
     **************************************************************************/
    TorqueManager::TorqueManager() :
        currentAngularVelocity{ 0.0f, 0.0f, 0.0f } {}

    /**************************************************************************
     * @brief
     * Copy-constructs, deep-copying owned torques and name maps.
     *
     * @param other
     * Source manager.
     **************************************************************************/
    TorqueManager::TorqueManager(TorqueManager const &other) :
        currentAngularVelocity{ other.currentAngularVelocity },
        maskToName{ other.maskToName },
        nameToMask{ other.nameToMask } {
        for(auto const &t : other.torques) {
            torques.push_back(t->Clone());
        }
    }

    /**************************************************************************
     * @brief
     * Move-constructs, transferring ownership.
     *
     * @param other
     * Source manager.
     **************************************************************************/
    TorqueManager::TorqueManager(TorqueManager &&other) noexcept :
        torques{ std::move(other.torques) },
        currentAngularVelocity{ other.currentAngularVelocity },
        maskToName{ std::move(other.maskToName) },
        nameToMask{ std::move(other.nameToMask) } {}

    /**************************************************************************
     * @brief
     * Copy assignment; deep-copies torques and state.
     *
     * @param other
     * Source manager.
     *
     * @return
     * Reference to this.
     **************************************************************************/
    TorqueManager &TorqueManager::operator=(TorqueManager const &other) {
        if(this != &other) {
            torques.clear();
            currentAngularVelocity = other.currentAngularVelocity;
            maskToName = other.maskToName;
            nameToMask = other.nameToMask;
            for(auto const &t : other.torques) {
                torques.push_back(t->Clone());
            }
        }
        return *this;
    }

    /**************************************************************************
     * @brief
     * Move assignment; transfers ownership.
     *
     * @param other
     * Source manager.
     *
     * @return
     * Reference to this.
     **************************************************************************/
    TorqueManager &TorqueManager::operator=(TorqueManager &&other) noexcept {
        if(this != &other) {
            torques = std::move(other.torques);
            currentAngularVelocity = other.currentAngularVelocity;
            maskToName = std::move(other.maskToName);
            nameToMask = std::move(other.nameToMask);
        }
        return *this;
    }

    /**************************************************************************
     * @brief
     * Adds a torque by cloning the given instance.
     *
     * @param t
     * Source torque to clone and store.
     **************************************************************************/
    void TorqueManager::AddTorque(Torque const &t) {
        torques.push_back(t.Clone());
    }

    /**************************************************************************
     * @brief
     * Returns the number of active torques.
     *
     * @return
     * Count of active torques.
     **************************************************************************/
    size_t TorqueManager::GetActiveTorqueCount() const {
        return static_cast<size_t>(std::count_if(
            torques.begin(), torques.end(),
            [](std::unique_ptr<Torque> const &t) { return t->GetIsActive(); }));
    }

    /**************************************************************************
     * @brief
     * Returns the total number of torques.
     *
     * @return
     * Count of all torques.
     **************************************************************************/
    size_t TorqueManager::GetTotalTorqueCount() const {
        return torques.size();
    }

    /**************************************************************************
     * @brief
     * Removes inactive Temp torques; preserves Perm torques.
     **************************************************************************/
    void TorqueManager::CleanupTorques() {
        torques.erase(
            std::remove_if(
            torques.begin(), torques.end(),
            [](std::unique_ptr<Torque> const &t) {
                return (!t->GetIsActive() && t->GetDurationType() == Torque::Temp);
            }),
            torques.end());
    }

    /**************************************************************************
     * @brief
     * Removes torques whose masks intersect the given mask.
     *
     * @param mask
     * Filter mask.
     **************************************************************************/
    void TorqueManager::RemoveTorquesByMask(unsigned mask) {
        torques.erase(
            std::remove_if(
            torques.begin(), torques.end(),
            [mask](std::unique_ptr<Torque> const &t) {
                return (t->GetTorqueMask() & mask) != 0u;
            }),
            torques.end());
    }

    /**************************************************************************
     * @brief
     * Sets activation for torques matching a mask.
     *
     * @param mask
     * Filter mask.
     * @param active
     * New activation state.
     **************************************************************************/
    void TorqueManager::ActivateByMask(unsigned mask, bool active) {
        for(auto &t : torques) {
            if((t->GetTorqueMask() & mask) != 0u) {
                t->SetIsActive(active);
            }
        }
    }

    /**************************************************************************
     * @brief
     * Sets lifetime for torques matching a mask (Temp only).
     *
     * @param mask
     * Filter mask.
     * @param lifetime
     * New lifetime.
     **************************************************************************/
    void TorqueManager::SetLifeTimeByMask(unsigned mask, float lifetime) {
        for(auto &t : torques) {
            if((t->GetTorqueMask() & mask) != 0u) {
                t->SetLifetime(lifetime);
            }
        }
    }

    /**************************************************************************
     * @brief
     * Accumulates torques that are active and match the given mask.
     *
     * @param mask
     * Filter mask.
     *
     * @return
     * Total torque vector.
     **************************************************************************/
    Vector3D TorqueManager::CalculateTorqueByMask(unsigned mask) const {
        Vector3D total{ 0.0f, 0.0f, 0.0f };
        for(auto const &t : torques) {
            if(t->GetIsActive() && (t->GetTorqueMask() & mask) != 0u) {
                Vector3D const tau{ t->CalculateTorque(currentAngularVelocity) };
                total.x += tau.x; total.y += tau.y; total.z += tau.z;
            }
        }
        return total;
    }

    /**************************************************************************
     * @brief
     * Accumulates all active torques regardless of mask.
     *
     * @return
     * Total torque vector.
     **************************************************************************/
    Vector3D TorqueManager::GetTotalTorque() const {
        Vector3D total{ 0.0f, 0.0f, 0.0f };
        for(auto const &t : torques) {
            if(t->GetIsActive()) {
                Vector3D const tau{ t->CalculateTorque(currentAngularVelocity) };
                total.x += tau.x; total.y += tau.y; total.z += tau.z;
            }
        }
        return total;
    }

    /**************************************************************************
     * @brief
     * Sets the current angular velocity used by velocity-dependent torques.
     *
     * @param w
     * Current angular velocity.
     **************************************************************************/
    void TorqueManager::SetCurrentAngularVelocity(Vector3D const &w) {
        currentAngularVelocity = w;
    }

    /**************************************************************************
     * @brief
     * Returns the current angular velocity sample.
     *
     * @return
     * Reference to current angular velocity.
     **************************************************************************/
    Vector3D const &TorqueManager::GetCurrentAngularVelocity() const {
        return currentAngularVelocity;
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
    void TorqueManager::RegisterMaskName(unsigned mask, std::string const &name) {
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
    bool TorqueManager::UnregisterMaskName(unsigned mask) {
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
    std::string TorqueManager::GetMaskName(unsigned mask) const {
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
    unsigned TorqueManager::FindMaskByName(std::string const &name) const {
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
    std::vector<std::pair<unsigned, std::string>> TorqueManager::ListMaskNames() const {
        std::vector<std::pair<unsigned, std::string>> out{};
        out.reserve(maskToName.size());
        for(auto const &kv : maskToName) {
            out.emplace_back(kv.first, kv.second);
        }
        return out;
    }

    /**************************************************************************
     * @brief
     * Accumulates torques matching the given mask name.
     *
     * @param name
     * Registered mask name.
     *
     * @return
     * Total torque vector, or zero if name not found.
     **************************************************************************/
    Vector3D TorqueManager::CalculateTorqueByMaskName(std::string const &name) const {
        unsigned const mask{ FindMaskByName(name) };
        if(mask == 0u) {
            return Vector3D{ 0.0f, 0.0f, 0.0f };
        }
        return CalculateTorqueByMask(mask);
    }

    /**************************************************************************
     * @brief
     * Returns the internal torque container (const).
     *
     * @return
     * Const reference to owned torques.
     **************************************************************************/
    std::vector<std::unique_ptr<Torque>> const &TorqueManager::GetTorques() const {
        return torques;
    }

} // namespace gam300
