#pragma once
#ifndef __TORQUE_MANAGER_H__
#define __TORQUE_MANAGER_H__

#include <algorithm>
#include <vector>
#include <memory>
#include <unordered_map>
#include <string>
#include "../Utility/Vector3D.h"

namespace gam300
{
    class Torque
    {
    public:
        enum DURATION { Temp, Perm, Impulse };

    protected:
        DURATION durationType;
        float    lifetime;
        bool     isActive;
        float    age;
        unsigned torqueMask;

    public:
        Torque(DURATION type, unsigned mask);
        virtual ~Torque() = default;

        float    GetLifetime() const;
        void     SetLifetime(float value);

        bool     GetIsActive() const;
        void     SetIsActive(bool value);

        float    GetAge() const;
        void     SetAge(float value);

        unsigned GetTorqueMask() const;
        DURATION GetDurationType() const;

        bool     ValidateAge();
        void     Update(double deltaTime);

        virtual Vector3D CalculateTorque(const Vector3D &currentAngularVelocity) const = 0;
        virtual std::unique_ptr<Torque> Clone() const = 0;
    };

    class AngularDirectionalTorque : public Torque
    {
    private:
        Vector3D unitAxis;   // must be unit
        float    magnitude;  // N·m

    public:
        AngularDirectionalTorque(const Vector3D &axisUnit, float magnitude,
            unsigned mask, DURATION type);

        Vector3D GetUnitAxis() const;
        void     SetUnitAxis(const Vector3D &axis);

        float    GetMagnitude() const;
        void     SetMagnitude(float value);

        Vector3D CalculateTorque(const Vector3D &currentAngularVelocity) const override;
        std::unique_ptr<Torque> Clone() const override;
    };

    class AngularDrag : public Torque
    {
    private:
        float dragX;
        float dragY;
        float dragZ;

    public:
        AngularDrag(float cx, float cy, float cz, unsigned mask,
            DURATION type = DURATION::Perm);

        float GetDragX() const;
        float GetDragY() const;
        float GetDragZ() const;
        void  SetDragX(float v);
        void  SetDragY(float v);
        void  SetDragZ(float v);

        Vector3D CalculateTorque(const Vector3D &currentAngularVelocity) const override;
        std::unique_ptr<Torque> Clone() const override;
    };

    class TorqueManager
    {
    private:
        std::vector<std::unique_ptr<Torque>> torques;
        Vector3D currentAngularVelocity;

        // mask <-> name registries (optional convenience, mirrors ForceManager)
        std::unordered_map<unsigned, std::string> maskToName;
        std::unordered_map<std::string, unsigned> nameToMask;

    public:
        TorqueManager();
        ~TorqueManager() = default;

        TorqueManager(const TorqueManager &other);
        TorqueManager(TorqueManager &&other) noexcept;
        TorqueManager &operator=(const TorqueManager &other);
        TorqueManager &operator=(TorqueManager &&other) noexcept;

        template<typename T, typename... Args>
        void AddTorque(Args&&... args)
        {
            torques.push_back(std::make_unique<T>(std::forward<Args>(args)...));
        }

        void   AddTorque(const Torque &t);

        size_t GetActiveTorqueCount() const;
        size_t GetTotalTorqueCount() const;

        void   CleanupTorques();

        // mask-based controls ((t.mask & mask) != 0)
        void   RemoveTorquesByMask(unsigned mask);
        void   ActivateByMask(unsigned mask, bool active);
        void   SetLifeTimeByMask(unsigned mask, float lifetime);

        // sums
        Vector3D CalculateTorqueByMask(unsigned mask) const;
        Vector3D GetTotalTorque() const;

        // angular velocity for drag-like torques
        void          SetCurrentAngularVelocity(const Vector3D &w);
        const Vector3D &GetCurrentAngularVelocity() const;

        // mask <-> name helpers
        void        RegisterMaskName(unsigned mask, const std::string &name);
        bool        UnregisterMaskName(unsigned mask);
        std::string GetMaskName(unsigned mask) const;
        unsigned    FindMaskByName(const std::string &name) const;
        std::vector<std::pair<unsigned, std::string>> ListMaskNames() const;
        Vector3D    CalculateTorqueByMaskName(const std::string &name) const;
        const std::vector<std::unique_ptr<Torque>> &GetTorques() const;

    };
}

#endif // __TORQUE_MANAGER_H__
