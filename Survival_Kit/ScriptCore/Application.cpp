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
        auto addScript = GetFunctionPtr<bool(*)(int, const char*)>
            (
                "ScriptAPI",
                "ScriptAPI.EngineInterface",
                "AddScriptViaName"
            );
        auto executeUpdate = GetFunctionPtr<void(*)(void)>
            (
                "ScriptAPI",
                "ScriptAPI.EngineInterface",
                "ExecuteUpdate"
            );
        auto reloadScripts = GetFunctionPtr<void(*)(void)>
            (
                "ScriptAPI",
                "ScriptAPI.EngineInterface",
                "Reload"
            );
        // Step 2: Initialize
        std::cout << "INIT application..." << std::endl;

        init();
        //// Step 3: Add script to an entity
        addScript(0, "TestScript");
        std::cout << "Test script added" << std::endl;

         
        while (true)
        {
            if (GetKeyState(VK_ESCAPE) & 0x8000)
                break;

            // Step 4: Run the Update loop for our scripts
            if (GetKeyState(VK_SPACE) & 0x8000)
            {
                compileScriptAssembly();
                reloadScripts();
                addScript(0, "TestScript");
            }
            executeUpdate();
        }
        stopScriptEngine();
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

        // Get current executable directory
        std::string execPath(MAX_PATH, '\0');
        GetModuleFileNameA(nullptr, execPath.data(), MAX_PATH);
        PathRemoveFileSpecA(execPath.data());
        execPath.resize(std::strlen(execPath.data()));

        std::cout << "Executable path: " << execPath << std::endl;


        // Look for dotnet one level up (shared between Debug/Release)
        std::string sharedDotnetPath = execPath + "\\..\\dotnet\\dotnet.exe";
        std::wstring dotnetExePath;

        if (std::filesystem::exists(sharedDotnetPath))
        {
            dotnetExePath = std::filesystem::absolute(sharedDotnetPath).wstring();
            std::cout << "Using shared bundled .NET at: " << sharedDotnetPath << std::endl;
        }
        else
        {
            dotnetExePath = L"C:\\Program Files\\dotnet\\dotnet.exe";
            std::cout << "Using system .NET" << std::endl;
        }

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

    void Application::InitializeScripting()
    {
        std::cout << "Starting script engine..." << std::endl;

        // Initialize MonoBehaviour templates
        if (!InitializeTemplates())
        {
            std::cout << "Warning: Failed to initialize MonoBehaviour templates" << std::endl;
        }

        startScriptEngine();
        compileScriptAssembly();

        std::cout << "Getting function pointers..." << std::endl;
        initFunc = GetFunctionPtr<void(*)(void)>("ScriptAPI", "ScriptAPI.EngineInterface", "Init");
        addScriptFunc = GetFunctionPtr<bool(*)(int, const char*)>("ScriptAPI", "ScriptAPI.EngineInterface", "AddScriptViaName");
        executeUpdateFunc = GetFunctionPtr<void(*)(void)>("ScriptAPI", "ScriptAPI.EngineInterface", "ExecuteUpdate");
        reloadScriptsFunc = GetFunctionPtr<void(*)(void)>("ScriptAPI", "ScriptAPI.EngineInterface", "Reload");

        std::cout << "Initializing script system..." << std::endl;
        initFunc();

        std::cout << "Script system initialized successfully!" << std::endl;

        // Initialize file watching
        updateFileTimestamps();
        lastCheck = std::chrono::steady_clock::now();
        startFileWatcher();

        std::cout << "Script system and file watcher initialized successfully!" << std::endl;
    }

    bool Application::AddScript(int entityId, const char* scriptName)
    {
        if (addScriptFunc)
            return addScriptFunc(entityId, scriptName);
        return false;
    }

    void Application::UpdateScripts()
    {
        if (executeUpdateFunc)
            executeUpdateFunc();
    }

    void Application::ShutdownScripting()
    {
        stopFileWatcher();
        stopScriptEngine();

        // Reset all function pointers
        initFunc = nullptr;
        addScriptFunc = nullptr;
        executeUpdateFunc = nullptr;
        reloadScriptsFunc = nullptr;
    }

    void Application::ReloadScripts()
    {
        if (reloadScriptsFunc)
        {
            std::cout << "Reloading scripts..." << std::endl;
            compileScriptAssembly();
            reloadScriptsFunc();
            std::cout << "Scripts reloaded!" << std::endl;
        }
    }


    void Application::CheckAndReloadScripts()
    {
        // Option 1: Use the atomic flag from file watcher thread
        if (scriptsNeedReload.exchange(false))
        {
            std::cout << "Auto-reloading scripts due to file changes..." << std::endl;
            try
            {
                // Add a small delay to ensure file write is complete
                std::this_thread::sleep_for(std::chrono::milliseconds(100));

                compileScriptAssembly();
                if (reloadScriptsFunc)
                {
                    reloadScriptsFunc();
                }

                // Re-add your scripts here
                AddScript(0, "TestScript");

                std::cout << "Auto-reload completed!" << std::endl;
            }
            catch (const std::exception& e)
            {
                std::cout << "Auto-reload failed: " << e.what() << std::endl;
            }
        }

        // Option 2: Alternative polling approach (comment out if using file watcher thread)
        /*
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastCheck).count() > 500)
        {
            if (checkForScriptChanges())
            {
                std::cout << "Auto-reloading scripts due to file changes..." << std::endl;
                try
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    compileScriptAssembly();
                    if (reloadScriptsFunc)
                    {
                        reloadScriptsFunc();
                    }
                    AddScript(0, "TestScript");
                    std::cout << "Auto-reload completed!" << std::endl;
                }
                catch (const std::exception& e)
                {
                    std::cout << "Auto-reload failed: " << e.what() << std::endl;
                }
            }
            lastCheck = now;
        }
        */
    }


    void Application::startFileWatcher()
    {
        scriptDirectory = "..\\..\\ManagedScripts\\";
        shouldStopWatching = false;
        fileWatcherThread = std::thread(&Application::fileWatcherLoop, this);
    }

    void Application::stopFileWatcher()
    {
        shouldStopWatching = true;
        if (fileWatcherThread.joinable())
            fileWatcherThread.join();
    }

    void Application::fileWatcherLoop()
    {
        HANDLE hDir = CreateFileA(
            scriptDirectory.c_str(),
            FILE_LIST_DIRECTORY,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            nullptr,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
            nullptr
        );

        if (hDir == INVALID_HANDLE_VALUE)
        {
            std::cout << "Failed to open directory for monitoring: " << scriptDirectory << std::endl;
            return;
        }

        char buffer[1024];
        DWORD bytesReturned;
        OVERLAPPED overlapped = {};
        overlapped.hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

        std::cout << "File watcher started for: " << scriptDirectory << std::endl;

        while (!shouldStopWatching)
        {
            if (ReadDirectoryChangesW(
                hDir,
                buffer,
                sizeof(buffer),
                TRUE, // Watch subdirectories
                FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION,
                &bytesReturned,
                &overlapped,
                nullptr))
            {
                DWORD waitResult = WaitForSingleObject(overlapped.hEvent, 1000);

                if (waitResult == WAIT_OBJECT_0)
                {
                    FILE_NOTIFY_INFORMATION* info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(buffer);

                    do
                    {
                        std::wstring filename(info->FileName, info->FileNameLength / sizeof(wchar_t));

                        // Check if it's a C# file
                        if (filename.length() > 3 &&
                            filename.substr(filename.length() - 3) == L".cs")
                        {
                            std::wcout << L"Detected change in: " << filename << std::endl;
                            scriptsNeedReload = true;
                            break;
                        }

                        if (info->NextEntryOffset == 0)
                            break;

                        info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(
                            reinterpret_cast<char*>(info) + info->NextEntryOffset);

                    } while (true);
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        CloseHandle(overlapped.hEvent);
        CloseHandle(hDir);
        std::cout << "File watcher stopped" << std::endl;
    }

    bool Application::checkForScriptChanges()
    {
        bool hasChanges = false;

        try
        {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(scriptDirectory))
            {
                if (entry.is_regular_file() && entry.path().extension() == ".cs")
                {
                    std::string filepath = entry.path().string();
                    auto lastWriteTime = entry.last_write_time();

                    auto it = fileTimestamps.find(filepath);
                    if (it == fileTimestamps.end())
                    {
                        fileTimestamps[filepath] = lastWriteTime;
                        hasChanges = true;
                        std::cout << "New script file detected: " << filepath << std::endl;
                    }
                    else if (it->second != lastWriteTime)
                    {
                        it->second = lastWriteTime;
                        hasChanges = true;
                        std::cout << "Script file modified: " << filepath << std::endl;
                    }
                }
            }
        }
        catch (const std::filesystem::filesystem_error& e)
        {
            std::cout << "Error checking script files: " << e.what() << std::endl;
        }

        return hasChanges;
    }

    void Application::updateFileTimestamps()
    {
        scriptDirectory = "..\\..\\ManagedScripts\\";

        try
        {
            fileTimestamps.clear();
            for (const auto& entry : std::filesystem::recursive_directory_iterator(scriptDirectory))
            {
                if (entry.is_regular_file() && entry.path().extension() == ".cs")
                {
                    fileTimestamps[entry.path().string()] = entry.last_write_time();
                }
            }
            std::cout << "Initialized tracking for " << fileTimestamps.size() << " C# files" << std::endl;
        }
        catch (const std::filesystem::filesystem_error& e)
        {
            std::cout << "Error updating file timestamps: " << e.what() << std::endl;
        }
    }

    // Script Creation Methods
    bool Application::CreateMonoBehaviourScript(const std::string& scriptName)
    {
        std::cout << "Creating MonoBehaviour script via Application..." << std::endl;

        bool success = MonoBehaviour::CreateScript(scriptName);

        if (success)
        {
            std::cout << "MonoBehaviour script created, triggering recompilation..." << std::endl;

            try
            {
                compileScriptAssembly();
                std::cout << "Recompilation completed successfully." << std::endl;
            }
            catch (const std::exception& e)
            {
                std::cout << "Warning: Recompilation failed: " << e.what() << std::endl;
            }
        }

        return success;
    }

    bool Application::CreateScriptableObjectScript(const std::string& scriptName)
    {
        // TODO: Implement ScriptableObject creation
        std::cout << "ScriptableObject creation not yet implemented" << std::endl;
        return false;
    }

    bool Application::CreateScriptFromTemplate(const std::string& scriptName, const std::string& templateType)
    {
        if (templateType == "MonoBehaviour")
        {
            return CreateMonoBehaviourScript(scriptName);
        }
        else if (templateType == "ScriptableObject")
        {
            return CreateScriptableObjectScript(scriptName);
        }
        else
        {
            std::cout << "Unknown template type: " << templateType << std::endl;
            return false;
        }
    }

    std::string Application::GetTemplatesDirectory()
    {
        return MonoBehaviour::GetTemplatesDirectory();
    }

    bool Application::InitializeTemplates()
    {
        return MonoBehaviour::InitializeTemplates();
    }

    std::vector<std::string> Application::GetAvailableTemplateTypes()
    {
        return { "MonoBehaviour" }; // Will add ScriptableObject later
    }

    // Validation and utility methods
    bool Application::ValidateScriptName(const std::string& scriptName)
    {
        return MonoBehaviour::ValidateScriptName(scriptName);
    }

    std::string Application::GetManagedScriptsDirectory()
    {
        return MonoBehaviour::GetScriptsDirectory();
    }

    bool Application::DoesScriptExist(const std::string& scriptName)
    {
        return MonoBehaviour::DoesScriptExist(scriptName);
    }

    std::vector<std::string> Application::GetExistingScriptFiles()
    {
        return MonoBehaviour::GetExistingScripts();
    }

    void Application::ListExistingScripts()
    {
        std::cout << "\n=== Existing Scripts ===" << std::endl;
        auto scripts = MonoBehaviour::GetExistingScripts();

        if (scripts.empty())
        {
            std::cout << "No scripts found in " << MonoBehaviour::GetScriptsDirectory() << std::endl;
        }
        else
        {
            for (const auto& script : scripts)
            {
                std::cout << "  - " << script << ".cs" << std::endl;
            }
            std::cout << "Total: " << scripts.size() << " scripts" << std::endl;
        }
        std::cout << "========================\n" << std::endl;
    }

    void Application::ShowScriptCreationHelp()
    {
        std::cout << "\n=== Script Creation Help ===" << std::endl;
        std::cout << "Available script types:" << std::endl;
        std::cout << "  1. MonoBehaviour - Components that attach to game objects" << std::endl;
        std::cout << "  2. ScriptableObject - Data containers (coming soon)" << std::endl;
        std::cout << "\nTemplate location: " << MonoBehaviour::GetTemplatesDirectory() << std::endl;
        std::cout << "Scripts location: " << MonoBehaviour::GetScriptsDirectory() << std::endl;
        std::cout << "============================\n" << std::endl;
    }

    bool Application::OpenScriptInEditor(const std::string& scriptName)
    {
        return MonoBehaviour::OpenScriptInEditor(scriptName);
    }


}