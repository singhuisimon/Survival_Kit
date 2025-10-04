#pragma once
#include "TransformComponent.hxx"
#include "MovementControllerComponent.hxx"  // ADD THIS

namespace ScriptAPI
{
    public ref class Script abstract
    {
    public:
        void virtual Update() {};
        TransformComponent GetTransformComponent();
        MovementControllerComponent GetMovementController();  // ADD THIS

    internal:
        void SetEntityId(int id);
    private:
        int entityId;
    };
}