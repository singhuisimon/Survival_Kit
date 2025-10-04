#pragma once

namespace ScriptAPI
{
    public value struct Vector3
    {
        float X;
        float Y;
        float Z;

        Vector3(float x, float y, float z) : X(x), Y(y), Z(z) {}
    };

    public value struct MovementControllerComponent
    {
    public:
        void SetDesiredDirection(Vector3 direction);
        Vector3 GetDesiredDirection();

        void SetDesiredRotation(Vector3 rotation);
        Vector3 GetDesiredRotation();

        void SetMoveForce(float force);
        float GetMoveForce();

        void SetKinematicSpeed(float speed);
        float GetKinematicSpeed();

        void SetRotationTorque(float torque);
        float GetRotationTorque();

    internal:
        MovementControllerComponent(int id);

    private:
        int entityId;
    };
}