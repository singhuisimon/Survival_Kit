#pragma once
#ifndef __FORCE_MANAGER_H__
#define __FORCE_MANAGER_H__

#include <algorithm>
#include <vector>
#include <utility>     
#include <memory>      
#include <unordered_map>
#include <string>
#include "../Utility/Vector3D.h"

namespace gam300
{
	class Force
	{
	public:
		enum DURATION { Temp, Perm, Impulse };

	protected:
		DURATION durationType;
		float    lifetime;
		bool     isActive;
		float    age;
		unsigned forceMask;  // bitmask categorization / selection

	public:
		Force(DURATION type, unsigned mask);
		virtual ~Force() = default;

		float    GetLifetime() const;
		void     SetLifetime(float value);

		bool     GetIsActive() const;
		void     SetIsActive(bool value);

		float    GetAge() const;
		void     SetAge(float value);

		unsigned GetForceMask() const;
		DURATION GetDurationType() const;

		bool     ValidateAge();
		void     Update(double deltaTime);

		virtual Vector3D CalculateForce(const Vector3D &currentVelocity) const = 0;
		virtual std::unique_ptr<Force> Clone() const = 0;
	};

	class LinearDirectionalForce : public Force
	{
	private:
		Vector3D unitDirection;
		float    magnitude;

	public:
		LinearDirectionalForce(const Vector3D &direction, float magnitude,
			unsigned mask, DURATION type);

		Vector3D GetUnitDirection() const;
		void     SetUnitDirection(const Vector3D &direction);

		float    GetMagnitude() const;
		void     SetMagnitude(float value);

		Vector3D CalculateForce(const Vector3D &currentVelocity) const override;
		std::unique_ptr<Force> Clone() const override;
	};

	class DragForce : public Force
	{
	private:
		float horizontalDragCoeff;
		float verticalDragCoeff;

	public:
		explicit DragForce(float hDrag, float vDrag, unsigned mask,
			DURATION type = DURATION::Perm);

		float GetHorizontalDragCoefficient() const;
		void  SetHorizontalDragCoefficient(float value);

		float GetVerticalDragCoefficient() const;
		// NOTE: original naming quirk kept: setter named like a getter.
		void  GetVerticalDragCoefficient(float value);

		Vector3D CalculateForce(const Vector3D &currentVelocity) const override;
		std::unique_ptr<Force> Clone() const override;
	};

	class ForceManager
	{
	private:
		std::vector<std::unique_ptr<Force>> forces;
		Vector3D currentVelocity;

		// NEW: mask <-> name registries
		std::unordered_map<unsigned, std::string> maskToName;
		std::unordered_map<std::string, unsigned> nameToMask;

	public:
		ForceManager();
		~ForceManager() = default;

		ForceManager(const ForceManager &other);
		ForceManager(ForceManager &&other) noexcept;
		ForceManager &operator=(const ForceManager &other);
		ForceManager &operator=(ForceManager &&other) noexcept;

		template<typename T, typename... Args>
		void AddForce(Args&&... args)
		{
			forces.push_back(std::make_unique<T>(std::forward<Args>(args)...));
		}

		// clone and add
		void   AddForce(const Force &force);

		size_t GetActiveForceCount() const;
		size_t GetTotalForceCount() const;

		void   CleanupForces();

		// mask-based controls ((f.mask & mask) != 0)
		void   RemoveForcesByMask(unsigned mask);
		void   ActivateByMask(unsigned mask, bool active);
		void   SetLifeTimeByMask(unsigned mask, float lifetime);

		// sums
		Vector3D CalculateForceByMask(unsigned mask) const;
		Vector3D GetTotalForce() const;

		// velocity source for drag, etc.
		void    SetCurrentVelocity(const Vector3D &velocity);
		const   Vector3D &GetCurrentVelocity() const;

		// Registers/overwrites the display name for a mask
		void        RegisterMaskName(unsigned mask, const std::string &name);
		// Removes mask<->name mapping (both directions)
		bool        UnregisterMaskName(unsigned mask);
		// Fetch name for a mask (returns empty string if none)
		std::string GetMaskName(unsigned mask) const;
		// Find a mask by name (returns 0 if not found)
		unsigned    FindMaskByName(const std::string &name) const;
		// List all pairs
		std::vector<std::pair<unsigned, std::string>> ListMaskNames() const;

		// Convenience: compute using a registered mask name (0 => no-op)
		Vector3D    CalculateForceByMaskName(const std::string &name) const;
	};
}

#endif // __FORCE_MANAGER_H__
