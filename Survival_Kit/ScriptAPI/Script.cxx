#include "Script.hxx"
#include "../ScriptCore/Application.h"

namespace ScriptAPI
{
    // Constructor - initialize the script
    Script::Script()
    {
        entityId = -1;
        isEnabled = true;
        awakeCalled = false;
        startCalled = false;
        objectName = "Script";
        objectTag = "Untagged";
    }

    // Component access
    TransformComponent^ Script::GetTransformComponent()
    {
        return gcnew TransformComponent(entityId);
    }

    void Script::SetEntityId(int id)
    {
        entityId = id;
    }

    // Utility methods
    bool Script::CompareTag(System::String^ tagToCompare)
    {
        return System::String::Equals(objectTag, tagToCompare, System::StringComparison::OrdinalIgnoreCase);
    }

    void Script::print(System::Object^ message)
    {
        System::Console::WriteLine(message);
    }

    // Internal lifecycle management - called by the engine
    void Script::InternalAwake()
    {
        if (!awakeCalled && isEnabled)
        {
            awakeCalled = true;
            Awake();
        }
    }

    void Script::InternalStart()
    {
        if (!startCalled && isEnabled && awakeCalled)
        {
            startCalled = true;
            Start();
        }
    }

    void Script::InternalUpdate()
    {
        if (isEnabled && awakeCalled)
        {
            // Ensure Start is called before Update
            if (!startCalled)
            {
                InternalStart();
            }
            Update();
        }
    }

    void Script::InternalFixedUpdate()
    {
        if (isEnabled && awakeCalled && startCalled)
        {
            FixedUpdate();
        }
    }

    void Script::InternalLateUpdate()
    {
        if (isEnabled && awakeCalled && startCalled)
        {
            LateUpdate();
        }
    }

    void Script::InternalDestroy()
    {
        if (awakeCalled)
        {
            OnDestroy();
        }
    }
}