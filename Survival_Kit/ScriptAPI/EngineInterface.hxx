// EngineInterface.hxx
#pragma once
#include "Script.hxx"
namespace ScriptAPI
{


    public ref class ScriptFieldInfo
    {
    public:
        System::String^ name;           // Field name (e.g., "moveSpeed")
        System::String^ typeName;       // Field type (e.g., "System.Single" = float)
        System::Object^ value;          // Current value
        bool isPublic;                  // Is the field public?
        bool hasSerializeField;         // Has [SerializeField] attribute?
        bool hasRange;                  // Has [Range] attribute?
        float rangeMin;                 // Min value (if hasRange)
        float rangeMax;                 // Max value (if hasRange)
        System::String^ tooltip;        // Tooltip text
        System::String^ displayName;    // Display name (or field name)

        ScriptFieldInfo()
        {
            name = nullptr;
            typeName = nullptr;
            value = nullptr;
            isPublic = false;
            hasSerializeField = false;
            hasRange = false;
            rangeMin = 0.0f;
            rangeMax = 0.0f;
            tooltip = System::String::Empty;
            displayName = System::String::Empty;
        }
    };

    // ref classes are classes in C#, value classes are structs in C#
    public ref class EngineInterface
    {
    public:
        static void HelloWorld();
        static void Init();
        static void Reload();
        static bool AddScriptViaName(int entityId, System::String^ scriptName);
        static void ExecuteUpdate();
        // NEW METHOD - Updates only specific entity's scripts

        static void ExecuteUpdateForEntity(int entityId);

        // NEW: Get all serializable fields from a script instance
        static System::Collections::Generic::List<ScriptFieldInfo^>^ GetScriptFields(int entityId);

        // NEW: Update a field value
        static bool SetScriptFieldValue(int entityId, System::String^ fieldName, System::Object^ value);

    private:
        using ScriptList = System::Collections::Generic::List<Script^>;
        static System::Runtime::Loader::AssemblyLoadContext^ loadContext;

        static System::Collections::Generic::List<ScriptList^>^ scripts;
        static System::Collections::Generic::IEnumerable<System::Type^>^ scriptTypeList;

        static void updateScriptTypeList();
    };

   


}