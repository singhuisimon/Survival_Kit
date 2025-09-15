#include "pch.h"
#include "Application.h"
#include <filesystem>

#pragma comment(lib, "shlwapi.lib")

// Rest of your code stays the same...

namespace Core
{

    std::array<TransformComponent, Application::ENTITY_COUNT> Application::nativeData;
    void Application::Run()
    {
        std::cout << "Starting application..." << std::endl;



        startScriptEngine();
        compileScriptAssembly();

        // Step 1: Get Functions
        auto init = GetFunctionPtr<void(*)(void)>
            (
                "ScriptAPI",
                "ScriptAPI.EngineInterface",
                "Init"
            );
        //auto addScript = GetFunctionPtr<bool(*)(int, const char*)>
        //    (
        //        "ScriptAPI",
        //        "ScriptAPI.EngineInterface",
        //        "AddScriptViaName"
        //    );
        //auto executeUpdate = GetFunctionPtr<void(*)(void)>
        //    (
        //        "ScriptAPI",
        //        "ScriptAPI.EngineInterface",
        //        "ExecuteUpdate"
        //    );
        //auto reloadScripts = GetFunctionPtr<void(*)(void)>
        //    (
        //        "ScriptAPI",
        //        "ScriptAPI.EngineInterface",
        //        "Reload"
        //    );
        // Step 2: Initialize
        std::cout << "INIT application..." << std::endl;

        init();
        //// Step 3: Add script to an entity
        //addScript(0, "TestScript");
        //std::cout << "Test script added" << std::endl;

        // Load
        //while (true)
        //{
        //    if (GetKeyState(VK_ESCAPE) & 0x8000)
        //        break;

        //    // Step 4: Run the Update loop for our scripts
        //    if (GetKeyState(VK_SPACE) & 0x8000)
        //    {
        //        compileScriptAssembly();
        //        reloadScripts();
        //        addScript(0, "TestScript");
        //    }
        //    executeUpdate();
        //}
        //stopScriptEngine();
    }
    void Application::HelloWorld()
    {
        std::cout << "Hello Native World!" << std::endl;
    }

    TransformComponent* Application::GetComponent(int entityId)
    {
        if (entityId < MIN_ENTITY_ID || entityId > MAX_ENTITY_ID)
            return nullptr;

        return &nativeData[entityId];
    }

    void Application::compileScriptAssembly()
    {
        const char* PROJ_PATH =
            "..\\..\\ManagedScripts\\ManagedScripts.csproj";
        std::wstring buildCmd = L" build \"" +
            std::filesystem::absolute(PROJ_PATH).wstring() +
            L"\" -c Debug --no-self-contained " +
            L"-o \"./tmp_build/\" -r \"win-x64\"";
        STARTUPINFOW startInfo;
        PROCESS_INFORMATION pi;
        ZeroMemory(&startInfo, sizeof(startInfo));
        ZeroMemory(&pi, sizeof(pi));
        startInfo.cb = sizeof(startInfo);
        // Start compiler process
        const auto SUCCESS = CreateProcess
        (
            L"C:\\Program Files\\dotnet\\dotnet.exe", buildCmd.data(),
            nullptr, nullptr, true, NULL, nullptr, nullptr,
            &startInfo, &pi
        );
        // Check that we launched the process
        if (!SUCCESS)
        {
            auto err = GetLastError();
            std::ostringstream oss;
            oss << "Failed to launch compiler. Error code: "
                << std::hex << err;
            throw std::runtime_error(oss.str());
        }
        // Wait for process to end
        DWORD exitCode{};
        while (true)
        {
            const auto EXEC_SUCCESS =
                GetExitCodeProcess(pi.hProcess, &exitCode);
            if (!EXEC_SUCCESS)
            {
                auto err = GetLastError();
                std::ostringstream oss;
                oss << "Failed to query process. Error code: "
                    << std::hex << err;
                throw std::runtime_error(oss.str());
            }
            if (exitCode != STILL_ACTIVE)
                break;
        }
        // Successful build
        if (exitCode == 0)
        {
            // Copy out files
            std::filesystem::copy_file
            (
                "./tmp_build/ManagedScripts.dll",
                "ManagedScripts.dll",
                std::filesystem::copy_options::overwrite_existing
            );
        }
        // Failed build
        else
        {
            throw std::runtime_error("Failed to build managed scripts!");
        }
    }
    void Application::startScriptEngine()
    {
        // Get the current executable directory so that we can find the coreclr.dll to load
        std::string runtimePath(MAX_PATH, '\0');
        GetModuleFileNameA(nullptr, runtimePath.data(), MAX_PATH);
        PathRemoveFileSpecA(runtimePath.data());
        // Since PathRemoveFileSpecA() removes from data(), the size is not updated, so we must manually update it
        runtimePath.resize(std::strlen(runtimePath.data()));
        std::filesystem::current_path(runtimePath);

        // Construct the CoreCLR path
        std::string coreClrPath(runtimePath); // Works
        coreClrPath += "\\coreclr.dll";

        // Load the CoreCLR DLL
        coreClr = LoadLibraryExA(coreClrPath.c_str(), nullptr, 0);
        if (!coreClr)
            throw std::runtime_error("Failed to load CoreCLR.");

        // Step 2: Get CoreCLR hosting functions
        initializeCoreClr = getCoreClrFuncPtr<coreclr_initialize_ptr>("coreclr_initialize");
        createManagedDelegate = getCoreClrFuncPtr<coreclr_create_delegate_ptr>("coreclr_create_delegate");
        shutdownCoreClr = getCoreClrFuncPtr<coreclr_shutdown_ptr>("coreclr_shutdown");

        // Step 3: Construct AppDomain properties used when starting the runtime
        std::string tpaList = buildTpaList(runtimePath);

        // Define CoreCLR properties
        std::array<const char*, 2> propertyKeys =
        {
            "TRUSTED_PLATFORM_ASSEMBLIES",
            "APP_PATHS",
        };
        std::array<const char*, 2> propertyValues =
        {
            tpaList.c_str(),
            runtimePath.c_str()
        };

        // Step 4: Start the CoreCLR runtime
        int result = initializeCoreClr
        (
            runtimePath.c_str(),     // AppDomain base path
            "SampleHost",            // AppDomain friendly name, this can be anything you want really
            propertyKeys.size(),     // Property count
            propertyKeys.data(),     // Property names
            propertyValues.data(),   // Property values
            &hostHandle,             // Host handle
            &domainId                // AppDomain ID
        );

        // Check if intiialization of CoreCLR failed
        if (result < 0)
        {
            std::ostringstream oss;
            oss << std::hex << std::setfill('0') << std::setw(8)
                << "Failed to initialize CoreCLR. Error 0x" << result << "\n";
            throw std::runtime_error(oss.str());
        }
    }
    void Application::stopScriptEngine()
    {
        // Shutdown CoreCLR
        const int RESULT = shutdownCoreClr(hostHandle, domainId);
        if (RESULT < 0)
        {
            std::stringstream oss;
            oss << std::hex << std::setfill('0') << std::setw(8)
                << "[DotNetRuntime] Failed to shut down CoreCLR. Error 0x" << RESULT << "\n";
            throw std::runtime_error(oss.str());
        }
    }
    std::string Application::buildTpaList(const std::string& directory)
    {
        // Constants
        static const std::string SEARCH_PATH = directory + "\\*.dll";
        static constexpr char PATH_DELIMITER = ';';

        // Create a osstream object to compile the string
        std::ostringstream tpaList;

        // Search the current directory for the TPAs (.DLLs)
        WIN32_FIND_DATAA findData;
        HANDLE fileHandle = FindFirstFileA(SEARCH_PATH.c_str(), &findData);
        if (fileHandle != INVALID_HANDLE_VALUE)
        {
            do
            {
                // Append the assembly to the list
                tpaList << directory << '\\' << findData.cFileName << PATH_DELIMITER;
            } while (FindNextFileA(fileHandle, &findData));
            FindClose(fileHandle);
        }

        return tpaList.str();
    }
}