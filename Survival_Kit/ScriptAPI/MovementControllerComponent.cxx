#include "MovementControllerComponent.hxx"
#include "../ScriptCore/Application.h"
namespace gam300
{
    // Minimal definition just for pointer casting
    struct Vector3D
    {
        float x, y, z;
        Vector3D(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
    };

    // Forward declaration only
    class MovementController;
}
namespace ScriptAPI
{
    MovementControllerComponent::MovementControllerComponent(int id)
        : entityId{ id }
    {
    }

    void MovementControllerComponent::SetDesiredDirection(Vector3 direction)
    {
        Core::Application::SetMovementControllerDirection(
            entityId, direction.X, direction.Y, direction.Z
        );
    }

    Vector3 MovementControllerComponent::GetDesiredDirection()
    {
        float x, y, z;
        Core::Application::GetMovementControllerDirection(entityId, &x, &y, &z);
        return Vector3(x, y, z);
    }

    void MovementControllerComponent::SetDesiredRotation(Vector3 rotation)
    {
        Core::Application::SetMovementControllerRotation(
            entityId, rotation.X, rotation.Y, rotation.Z
        );
    }

    Vector3 MovementControllerComponent::GetDesiredRotation()
    {
        float x, y, z;
        Core::Application::GetMovementControllerRotation(entityId, &x, &y, &z);
        return Vector3(x, y, z);
    }

    void MovementControllerComponent::SetMoveForce(float force)
    {
        Core::Application::SetMovementControllerMoveForce(entityId, force);
    }

    float MovementControllerComponent::GetMoveForce()
    {
        return Core::Application::GetMovementControllerMoveForce(entityId);
    }

    void MovementControllerComponent::SetKinematicSpeed(float speed)
    {
        Core::Application::SetMovementControllerKinematicSpeed(entityId, speed);
    }

    float MovementControllerComponent::GetKinematicSpeed()
    {
        return Core::Application::GetMovementControllerKinematicSpeed(entityId);
    }

    void MovementControllerComponent::SetRotationTorque(float torque)
    {
        Core::Application::SetMovementControllerRotationTorque(entityId, torque);
    }

    float MovementControllerComponent::GetRotationTorque()
    {
        return Core::Application::GetMovementControllerRotationTorque(entityId);
    }
}