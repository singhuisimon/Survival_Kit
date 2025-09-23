#pragma once
#include "TransformComponent.hxx"

namespace ScriptAPI
{
    public ref class Script abstract
    {
    public:
        Script();
        // Core lifecycle methods (virtual so they can be overridden)
        virtual void Awake() {}
        virtual void Start() {}
        virtual void Update() {}
        virtual void FixedUpdate() {}
        virtual void LateUpdate() {}
        virtual void OnDestroy() {}

        // Enable/Disable lifecycle
        virtual void OnEnable() {}
        virtual void OnDisable() {}

        // Common collision events
        virtual void OnCollisionEnter() {}
        virtual void OnCollisionExit() {}
        virtual void OnCollisionStay() {}

        // Trigger events
        virtual void OnTriggerEnter() {}
        virtual void OnTriggerExit() {}
        virtual void OnTriggerStay() {}

        // Mouse events
        virtual void OnMouseEnter() {}
        virtual void OnMouseExit() {}
        virtual void OnMouseDown() {}
        virtual void OnMouseUp() {}
        virtual void OnMouseOver() {}
        virtual void OnMouseDrag() {}

        // Component access
        TransformComponent^ GetTransformComponent();

        // Properties
        property bool enabled
        {
            bool get() { return isEnabled; }
            void set(bool value)
            {
                if (isEnabled != value)
                {
                    isEnabled = value;
                    if (value)
                        OnEnable();
                    else
                        OnDisable();
                }
            }
        }

        property System::String^ name
        {
            System::String^ get() { return objectName; }
            void set(System::String^ value) { objectName = value; }
        }

        property System::String^ tag
        {
            System::String^ get() { return objectTag; }
            void set(System::String^ value) { objectTag = value; }
        }

        // Utility methods
        bool CompareTag(System::String^ tagToCompare);

        // Static utility
        static void print(System::Object^ message);

    internal:
        void SetEntityId(int id);

        // Internal lifecycle management - called by the engine
        void InternalAwake();
        void InternalStart();
        void InternalUpdate();
        void InternalFixedUpdate();
        void InternalLateUpdate();
        void InternalDestroy();

        // State tracking properties
        property bool hasAwakeCalled
        {
            bool get() { return awakeCalled; }
        }

        property bool hasStartCalled
        {
            bool get() { return startCalled; }
        }

    private:
        int entityId;
        bool isEnabled;
        bool awakeCalled;
        bool startCalled;
        System::String^ objectName;
        System::String^ objectTag;
    };
}