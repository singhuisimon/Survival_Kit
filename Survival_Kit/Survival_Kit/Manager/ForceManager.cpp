#include "ForceManager.h"

namespace gam300
{
	static inline Vector3D NormalizeSafe(const Vector3D &v)
	{
		const float len2 = v.x * v.x + v.y * v.y + v.z * v.z;
		if (len2 <= 1e-12f) return Vector3D(0.0f, 0.0f, 0.0f);
		const float invLen = 1.0f / std::sqrt(len2);
		return Vector3D(v.x * invLen, v.y * invLen, v.z * invLen);
	}

	Force::Force(DURATION type, unsigned mask)
		: durationType(type), lifetime(0.0f), isActive(true), age(0.0f), forceMask(mask)
	{
		if (durationType == Perm) { lifetime = -1.0f; }
	}

	float    Force::GetLifetime() const { return lifetime; }
	void     Force::SetLifetime(float value) { if (durationType != Perm) lifetime = value; }

	bool     Force::GetIsActive() const { return isActive; }
	void     Force::SetIsActive(bool value) { isActive = value; }

	float    Force::GetAge() const { return age; }
	void     Force::SetAge(float value) { age = value; }

	unsigned Force::GetForceMask() const { return forceMask; }
	Force::DURATION Force::GetDurationType() const { return durationType; }

	bool Force::ValidateAge()
	{
		if (durationType != Perm && age >= lifetime)
		{
			isActive = false;
			return false;
		}
		return true;
	}

	void Force::Update(double deltaTime)
	{
		if (durationType != Perm && isActive)
		{
			age += static_cast<float>(deltaTime);
			if (!ValidateAge()) SetAge(0.0f);
		}
	}

	LinearDirectionalForce::LinearDirectionalForce(const Vector3D &direction,
		float mag,
		unsigned mask,
		DURATION type)
		: Force(type, mask), unitDirection(NormalizeSafe(direction)), magnitude(mag)
	{}

	Vector3D LinearDirectionalForce::GetUnitDirection() const { return unitDirection; }
	void     LinearDirectionalForce::SetUnitDirection(const Vector3D &direction)
	{
		unitDirection = NormalizeSafe(direction);
	}

	float    LinearDirectionalForce::GetMagnitude() const { return magnitude; }
	void     LinearDirectionalForce::SetMagnitude(float value) { magnitude = value; }

	Vector3D LinearDirectionalForce::CalculateForce(const Vector3D & /*currentVelocity*/) const
	{
		return Vector3D(unitDirection.x * magnitude,
			unitDirection.y * magnitude,
			unitDirection.z * magnitude);
	}

	std::unique_ptr<Force> LinearDirectionalForce::Clone() const
	{
		return std::make_unique<LinearDirectionalForce>(*this);
	}

	DragForce::DragForce(float hDrag, float vDrag, unsigned mask, DURATION type)
		: Force(type, mask), horizontalDragCoeff(hDrag), verticalDragCoeff(vDrag)
	{
		if (type == Perm) { lifetime = -1.0f; }
	}

	float DragForce::GetHorizontalDragCoefficient() const { return horizontalDragCoeff; }
	void  DragForce::SetHorizontalDragCoefficient(float value) { horizontalDragCoeff = value; }

	float DragForce::GetVerticalDragCoefficient() const { return verticalDragCoeff; }
	void  DragForce::GetVerticalDragCoefficient(float value) { verticalDragCoeff = value; }

	Vector3D DragForce::CalculateForce(const Vector3D &currentVelocity) const
	{
		// horizontal drag on XZ, vertical on Y
		return Vector3D(
			-currentVelocity.x * horizontalDragCoeff,
			-currentVelocity.y * verticalDragCoeff,
			-currentVelocity.z * horizontalDragCoeff
		);
	}

	std::unique_ptr<Force> DragForce::Clone() const
	{
		return std::make_unique<DragForce>(*this);
	}

    ForceManager::ForceManager()
        : currentVelocity(0.0f, 0.0f, 0.0f)
    {}

    ForceManager::ForceManager(const ForceManager &other)
        : currentVelocity(other.currentVelocity)
    {
        for (const auto &f : other.forces) forces.push_back(f->Clone());
    }

    ForceManager::ForceManager(ForceManager &&other) noexcept
        : forces(std::move(other.forces))
        , currentVelocity(other.currentVelocity)
    {}

    ForceManager &ForceManager::operator=(const ForceManager &other)
    {
        if (this != &other)
        {
            forces.clear();
            currentVelocity = other.currentVelocity;
            for (const auto &f : other.forces) forces.push_back(f->Clone());
        }
        return *this;
    }

    ForceManager &ForceManager::operator=(ForceManager &&other) noexcept
    {
        if (this != &other)
        {
            forces = std::move(other.forces);
            currentVelocity = other.currentVelocity;
        }
        return *this;
    }

    void ForceManager::AddForce(const Force &force)
    {
        forces.push_back(force.Clone());
    }

    size_t ForceManager::GetActiveForceCount() const
    {
        return static_cast<size_t>(std::count_if(forces.begin(), forces.end(),
            [](const std::unique_ptr<Force> &f) { return f->GetIsActive(); }));
    }

    size_t ForceManager::GetTotalForceCount() const
    {
        return forces.size();
    }

    void ForceManager::CleanupForces()
    {
        forces.erase(
            std::remove_if(forces.begin(), forces.end(),
                [](const std::unique_ptr<Force> &f)
                {
                    return (!f->GetIsActive() && f->GetDurationType() == Force::Temp);
                }),
            forces.end()
        );
    }

    void ForceManager::RemoveForcesByMask(unsigned mask)
    {
        forces.erase(
            std::remove_if(forces.begin(), forces.end(),
                [mask](const std::unique_ptr<Force> &f)
                {
                    return (f->GetForceMask() & mask) != 0u;
                }),
            forces.end()
        );
    }

    void ForceManager::ActivateByMask(unsigned mask, bool active)
    {
        for (auto &f : forces)
        {
            if ((f->GetForceMask() & mask) != 0u)
                f->SetIsActive(active);
        }
    }

    void ForceManager::SetLifeTimeByMask(unsigned mask, float lifetime)
    {
        for (auto &f : forces)
        {
            if ((f->GetForceMask() & mask) != 0u)
                f->SetLifetime(lifetime);
        }
    }

    Vector3D ForceManager::CalculateForceByMask(unsigned mask) const
    {
        Vector3D total(0.0f, 0.0f, 0.0f);
        for (const auto &f : forces)
        {
            if (f->GetIsActive() && (f->GetForceMask() & mask) != 0u)
            {
                const Vector3D c = f->CalculateForce(currentVelocity);
                total.x += c.x; total.y += c.y; total.z += c.z;
            }
        }
        return total;
    }

    Vector3D ForceManager::GetTotalForce() const
    {
        Vector3D total(0.0f, 0.0f, 0.0f);
        for (const auto &f : forces)
        {
            if (f->GetIsActive())
            {
                const Vector3D c = f->CalculateForce(currentVelocity);
                total.x += c.x; total.y += c.y; total.z += c.z;
            }
        }
        return total;
    }

    void ForceManager::SetCurrentVelocity(const Vector3D &velocity)
    {
        currentVelocity = velocity;
    }

    const Vector3D &ForceManager::GetCurrentVelocity() const
    {
        return currentVelocity;
    }

    void ForceManager::RegisterMaskName(unsigned mask, const std::string &name)
    {
        maskToName[mask] = name;
        nameToMask[name] = mask;
    }

    bool ForceManager::UnregisterMaskName(unsigned mask)
    {
        auto it = maskToName.find(mask);
        if (it == maskToName.end()) return false;
        const std::string old = it->second;
        maskToName.erase(it);
        auto it2 = nameToMask.find(old);
        if (it2 != nameToMask.end()) nameToMask.erase(it2);
        return true;
    }

    std::string ForceManager::GetMaskName(unsigned mask) const
    {
        auto it = maskToName.find(mask);
        return (it == maskToName.end()) ? std::string() : it->second;
    }

    unsigned ForceManager::FindMaskByName(const std::string &name) const
    {
        auto it = nameToMask.find(name);
        return (it == nameToMask.end()) ? 0u : it->second;
    }

    std::vector<std::pair<unsigned, std::string>> ForceManager::ListMaskNames() const
    {
        std::vector<std::pair<unsigned, std::string>> out;
        out.reserve(maskToName.size());
        for (const auto &kv : maskToName) out.emplace_back(kv.first, kv.second);
        return out;
    }

    Vector3D ForceManager::CalculateForceByMaskName(const std::string &name) const
    {
        const unsigned mask = FindMaskByName(name);
        if (mask == 0u) return Vector3D(0.0f, 0.0f, 0.0f);
        return CalculateForceByMask(mask);
    }

    std::vector<std::unique_ptr<Force>> const &ForceManager::GetForces() const {
        return forces;
    }


} // namespace gam300
