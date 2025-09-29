#include "TorqueManager.h"

namespace gam300
{
    static inline Vector3D NormalizeSafe(const Vector3D &v)
    {
        const float len2 = v.x * v.x + v.y * v.y + v.z * v.z;
        if (len2 <= 1e-12f) return Vector3D(0.0f, 0.0f, 0.0f);
        const float invLen = 1.0f / std::sqrt(len2);
        return Vector3D(v.x * invLen, v.y * invLen, v.z * invLen);
    }

    // Torque (base)
    Torque::Torque(DURATION type, unsigned mask)
        : durationType(type), lifetime(0.0f), isActive(true), age(0.0f), torqueMask(mask)
    {
        if (durationType == Perm) lifetime = -1.0f;
    }

    float Torque::GetLifetime() const { return lifetime; }
    void  Torque::SetLifetime(float value) { if (durationType != Perm) lifetime = value; }

    bool  Torque::GetIsActive() const { return isActive; }
    void  Torque::SetIsActive(bool value) { isActive = value; }

    float Torque::GetAge() const { return age; }
    void  Torque::SetAge(float value) { age = value; }

    unsigned Torque::GetTorqueMask() const { return torqueMask; }
    Torque::DURATION Torque::GetDurationType() const { return durationType; }

    bool Torque::ValidateAge()
    {
        if (durationType != Perm && age >= lifetime)
        {
            isActive = false;
            return false;
        }
        return true;
    }

    void Torque::Update(double deltaTime)
    {
        if (durationType != Perm && isActive)
        {
            age += static_cast<float>(deltaTime);
            if (!ValidateAge()) SetAge(0.0f);
        }
    }

    // AngularDirectionalTorque
    AngularDirectionalTorque::AngularDirectionalTorque(const Vector3D &axis,
        float mag,
        unsigned mask,
        DURATION type)
        : Torque(type, mask), unitAxis(NormalizeSafe(axis)), magnitude(mag)
    {}

    Vector3D AngularDirectionalTorque::GetUnitAxis() const { return unitAxis; }
    void     AngularDirectionalTorque::SetUnitAxis(const Vector3D &axis)
    {
        unitAxis = NormalizeSafe(axis);
    }

    float    AngularDirectionalTorque::GetMagnitude() const { return magnitude; }
    void     AngularDirectionalTorque::SetMagnitude(float value) { magnitude = value; }

    Vector3D AngularDirectionalTorque::CalculateTorque(const Vector3D & /*w*/) const
    {
        return Vector3D(unitAxis.x * magnitude,
            unitAxis.y * magnitude,
            unitAxis.z * magnitude);
    }

    std::unique_ptr<Torque> AngularDirectionalTorque::Clone() const
    {
        return std::make_unique<AngularDirectionalTorque>(*this);
    }

    // AngularDrag
    AngularDrag::AngularDrag(float cx, float cy, float cz, unsigned mask, DURATION type)
        : Torque(type, mask), dragX(cx), dragY(cy), dragZ(cz)
    {
        if (type == Perm) lifetime = -1.0f;
    }

    float AngularDrag::GetDragX() const { return dragX; }
    float AngularDrag::GetDragY() const { return dragY; }
    float AngularDrag::GetDragZ() const { return dragZ; }
    void  AngularDrag::SetDragX(float v) { dragX = v; }
    void  AngularDrag::SetDragY(float v) { dragY = v; }
    void  AngularDrag::SetDragZ(float v) { dragZ = v; }

    Vector3D AngularDrag::CalculateTorque(const Vector3D &w) const
    {
        return Vector3D(-dragX * w.x, -dragY * w.y, -dragZ * w.z);
    }

    std::unique_ptr<Torque> AngularDrag::Clone() const
    {
        return std::make_unique<AngularDrag>(*this);
    }

    // TorqueManager
    TorqueManager::TorqueManager()
        : currentAngularVelocity(0.0f, 0.0f, 0.0f)
    {}

    TorqueManager::TorqueManager(const TorqueManager &other)
        : currentAngularVelocity(other.currentAngularVelocity)
        , maskToName(other.maskToName)
        , nameToMask(other.nameToMask)
    {
        for (const auto &t : other.torques) torques.push_back(t->Clone());
    }

    TorqueManager::TorqueManager(TorqueManager &&other) noexcept
        : torques(std::move(other.torques))
        , currentAngularVelocity(other.currentAngularVelocity)
        , maskToName(std::move(other.maskToName))
        , nameToMask(std::move(other.nameToMask))
    {}

    TorqueManager &TorqueManager::operator=(const TorqueManager &other)
    {
        if (this != &other)
        {
            torques.clear();
            currentAngularVelocity = other.currentAngularVelocity;
            maskToName = other.maskToName;
            nameToMask = other.nameToMask;
            for (const auto &t : other.torques) torques.push_back(t->Clone());
        }
        return *this;
    }

    TorqueManager &TorqueManager::operator=(TorqueManager &&other) noexcept
    {
        if (this != &other)
        {
            torques = std::move(other.torques);
            currentAngularVelocity = other.currentAngularVelocity;
            maskToName = std::move(other.maskToName);
            nameToMask = std::move(other.nameToMask);
        }
        return *this;
    }

    void TorqueManager::AddTorque(const Torque &t)
    {
        torques.push_back(t.Clone());
    }

    size_t TorqueManager::GetActiveTorqueCount() const
    {
        return static_cast<size_t>(std::count_if(torques.begin(), torques.end(),
            [](const std::unique_ptr<Torque> &t) { return t->GetIsActive(); }));
    }

    size_t TorqueManager::GetTotalTorqueCount() const
    {
        return torques.size();
    }

    void TorqueManager::CleanupTorques()
    {
        torques.erase(
            std::remove_if(torques.begin(), torques.end(),
                [](const std::unique_ptr<Torque> &t)
                {
                    return (!t->GetIsActive() && t->GetDurationType() == Torque::Temp);
                }),
            torques.end()
        );
    }

    void TorqueManager::RemoveTorquesByMask(unsigned mask)
    {
        torques.erase(
            std::remove_if(torques.begin(), torques.end(),
                [mask](const std::unique_ptr<Torque> &t)
                {
                    return (t->GetTorqueMask() & mask) != 0u;
                }),
            torques.end()
        );
    }

    void TorqueManager::ActivateByMask(unsigned mask, bool active)
    {
        for (auto &t : torques)
        {
            if ((t->GetTorqueMask() & mask) != 0u) t->SetIsActive(active);
        }
    }

    void TorqueManager::SetLifeTimeByMask(unsigned mask, float lifetime)
    {
        for (auto &t : torques)
        {
            if ((t->GetTorqueMask() & mask) != 0u) t->SetLifetime(lifetime);
        }
    }

    Vector3D TorqueManager::CalculateTorqueByMask(unsigned mask) const
    {
        Vector3D total(0.0f, 0.0f, 0.0f);
        for (const auto &t : torques)
        {
            if (t->GetIsActive() && (t->GetTorqueMask() & mask) != 0u)
            {
                const Vector3D tau = t->CalculateTorque(currentAngularVelocity);
                total.x += tau.x; total.y += tau.y; total.z += tau.z;
            }
        }
        return total;
    }

    Vector3D TorqueManager::GetTotalTorque() const
    {
        Vector3D total(0.0f, 0.0f, 0.0f);
        for (const auto &t : torques)
        {
            if (t->GetIsActive())
            {
                const Vector3D tau = t->CalculateTorque(currentAngularVelocity);
                total.x += tau.x; total.y += tau.y; total.z += tau.z;
            }
        }
        return total;
    }

    void TorqueManager::SetCurrentAngularVelocity(const Vector3D &w)
    {
        currentAngularVelocity = w;
    }

    const Vector3D &TorqueManager::GetCurrentAngularVelocity() const
    {
        return currentAngularVelocity;
    }

    void TorqueManager::RegisterMaskName(unsigned mask, const std::string &name)
    {
        maskToName[mask] = name;
        nameToMask[name] = mask;
    }

    bool TorqueManager::UnregisterMaskName(unsigned mask)
    {
        auto it = maskToName.find(mask);
        if (it == maskToName.end()) return false;
        const std::string old = it->second;
        maskToName.erase(it);
        auto it2 = nameToMask.find(old);
        if (it2 != nameToMask.end()) nameToMask.erase(it2);
        return true;
    }

    std::string TorqueManager::GetMaskName(unsigned mask) const
    {
        auto it = maskToName.find(mask);
        return (it == maskToName.end()) ? std::string() : it->second;
    }

    unsigned TorqueManager::FindMaskByName(const std::string &name) const
    {
        auto it = nameToMask.find(name);
        return (it == nameToMask.end()) ? 0u : it->second;
    }

    std::vector<std::pair<unsigned, std::string>> TorqueManager::ListMaskNames() const
    {
        std::vector<std::pair<unsigned, std::string>> out;
        out.reserve(maskToName.size());
        for (const auto &kv : maskToName) out.emplace_back(kv.first, kv.second);
        return out;
    }

    Vector3D TorqueManager::CalculateTorqueByMaskName(const std::string &name) const
    {
        const unsigned mask = FindMaskByName(name);
        if (mask == 0u) return Vector3D(0.0f, 0.0f, 0.0f);
        return CalculateTorqueByMask(mask);
    }
}
