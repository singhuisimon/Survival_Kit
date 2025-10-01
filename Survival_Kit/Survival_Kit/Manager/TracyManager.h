#pragma once
#ifndef __TRACY_MANAGER_H__
#define __TRACY_MANAGER_H__

#include <string>
#include "../Utility/AssetPath.h"
#include "../Manager/LogManager.h"

#include "Manager.h"

#ifdef _WIN32
#include <windows.h>
#endif

#define TRACY gam300::TracyManager::getInstance()

namespace gam300 {

    class TracyManager : public Manager{
    public:
        // Singleton access
        static TracyManager& getInstance();

        // Set the path to Tracy executable
        void setTracyPath(const std::string& exename);

        // Launch Tracy profiler
        void launchTracy();

        // Detect when Tracy GUI closes
        void update();

        // Startup/ Shutdown override
        int startUp() override;
        void shutDown() override;

        // Check if Tracy has been launched already
        bool isRunning() const { return m_running; }

    private:
        TracyManager();
        ~TracyManager();

        TracyManager(const TracyManager&) = delete;
        TracyManager& operator=(const TracyManager&) = delete;

        std::string m_tracyPath;
        bool m_running = false;
        HANDLE m_processHandle;
    };

} // namespace gam300

#endif // __TRACY_MANAGER_H__
