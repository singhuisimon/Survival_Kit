#include "ScriptExports.h"
#include "../Component/MovementController.h"
#include "../Manager/ECSManager.h"
#include "../Utility/Vector3D.h"

extern "C" {

    void SetMovementDirection(int entityId, float x, float y, float z)
    {
        auto* controller = gam300::ECSManager::getInstance()
            .getComponent<gam300::MovementController>(entityId);

        if (controller) {
            controller->setDesiredDirection(gam300::Vector3D(x, y, z));
        }
    }

    void GetMovementDirection(int entityId, float* x, float* y, float* z)
    {
        auto* controller = gam300::ECSManager::getInstance()
            .getComponent<gam300::MovementController>(entityId);

        if (controller) {
            const gam300::Vector3D& dir = controller->getDesiredDirection();
            *x = dir.x;
            *y = dir.y;
            *z = dir.z;
        }
        else {
            *x = *y = *z = 0.0f;
        }
    }

    void SetMovementRotation(int entityId, float x, float y, float z)
    {
        auto* controller = gam300::ECSManager::getInstance()
            .getComponent<gam300::MovementController>(entityId);

        if (controller) {
            controller->setDesiredRotation(gam300::Vector3D(x, y, z));
        }
    }

    void GetMovementRotation(int entityId, float* x, float* y, float* z)
    {
        auto* controller = gam300::ECSManager::getInstance()
            .getComponent<gam300::MovementController>(entityId);

        if (controller) {
            const gam300::Vector3D& rot = controller->getDesiredRotation();
            *x = rot.x;
            *y = rot.y;
            *z = rot.z;
        }
        else {
            *x = *y = *z = 0.0f;
        }
    }

    void SetMovementMoveForce(int entityId, float force)
    {
        auto* controller = gam300::ECSManager::getInstance()
            .getComponent<gam300::MovementController>(entityId);

        if (controller) {
            controller->setMoveForce(force);
        }
    }

    float GetMovementMoveForce(int entityId)
    {
        auto* controller = gam300::ECSManager::getInstance()
            .getComponent<gam300::MovementController>(entityId);

        return controller ? controller->getMoveForce() : 0.0f;
    }

    void SetMovementKinematicSpeed(int entityId, float speed)
    {
        auto* controller = gam300::ECSManager::getInstance()
            .getComponent<gam300::MovementController>(entityId);

        if (controller) {
            controller->setKinematicSpeed(speed);
        }
    }

    float GetMovementKinematicSpeed(int entityId)
    {
        auto* controller = gam300::ECSManager::getInstance()
            .getComponent<gam300::MovementController>(entityId);

        return controller ? controller->getKinematicSpeed() : 0.0f;
    }

    void SetMovementRotationTorque(int entityId, float torque)
    {
        auto* controller = gam300::ECSManager::getInstance()
            .getComponent<gam300::MovementController>(entityId);

        if (controller) {
            controller->setRotationTorque(torque);
        }
    }

    float GetMovementRotationTorque(int entityId)
    {
        auto* controller = gam300::ECSManager::getInstance()
            .getComponent<gam300::MovementController>(entityId);

        return controller ? controller->getRotationTorque() : 0.0f;
    }

} // extern "C"